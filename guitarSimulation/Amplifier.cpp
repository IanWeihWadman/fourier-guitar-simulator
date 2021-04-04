#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>
#include <thread>
#include "Amplifier.h"


Amplifier::Amplifier(std::string File1, std::string File2, std::string File3, std::string File4, std::string File5, std::string File6, std::string outFile, std::string presetFile, int totalLength) : sync{ 32 }
{
	timeStep = 1 / (double)(44100);
	std::ifstream presetStream("guitarSimulation\\amppresets\\" + presetFile + ".txt");
	if (presetStream.fail())
	{
		throw std::exception("No amp preset\n");
	}
	presetStream >> bandClip;
	presetStream >> compress;
	presetStream >> threshold;
	presetStream >> gain;
	presetStream >> linearGain;
	presetStream >> hardClip;
	presetStream >> linearHardClip;
	presetStream >> volumeFollow;
	presetStream >> bandCompress;
	presetStream >> lowestFreq;
	presetStream >> linearSpread;
	presetStream >> quadraticSpread;
	presetStream >> bands;
	presetStream >> comb;
	presetStream >> combWidth;
	presetStream >> postGain;
	presetStream >> linearMix;
	presetStream >> feedbackPower;
	presetStream >> feedbackWavelength;
	presetStream.close();
	for (int i = 0; i < 32 - bands - 1; i++) {
		//Hack to redefine barrier maximum after initialization, due to unavailability of band count prior to the reading of the ampPreset. Maybe make separate preset reader class?
		sync.arrive_and_drop();
	}
	std::thread mainThread(&Amplifier::processOutput, this, File1, File2, File3, File4, File5, File6, outFile, totalLength);
	std::vector<std::thread> threadVector;
	for (int k = 0; k < bands; k++) {
		threadVector.emplace_back(&Amplifier::processBand, this, k, totalLength);
	}
	for (int k = 0; k < bands; k++) {
		threadVector[k].join();
	}
	mainThread.join();
	//if (output.fail()) { throw std::exception("Failed to open wave output file\n"); }
}

void Amplifier::processBand(int band, int totalLength) {
	std::deque<double> preGainWindow;
	std::deque<double> postGainWindow;
	std::vector<double> filterProfile;
	filterProfile.push_back(3.1415 * timeStep * 2000 * pow(2, band));
	for (int j = 1; j < 5000; j++) {
		filterProfile.push_back(sin(3.1415 * timeStep * j * 15000 * pow(2, band) / pow(2, bands)) * cos(3.1415 * pow(2, lowestFreq + (double)quadraticSpread * band * band + (double)linearSpread * band) * timeStep * j) / j);
	}
	preGainWindow.assign(filterWindow, 0);
	postGainWindow.assign(filterWindow, 0);
	double preAmplitude = 0;
	double postAmplitude = 0;
	for (int i = 0; i < totalLength + 22050; i++) {
		double splitValue = 0;
		sync.arrive_and_wait();
		for (int j = 0; j < 5000; j++) {
			splitValue += window[j] * filterProfile[j];
		}
		preGainWindow.push_front(splitValue);
		preGainWindow.pop_back();
		preAmplitude += preGainWindow[0] * preGainWindow[0] / (1 + bandCompress * preGainWindow[0] * preGainWindow[0]);
		preAmplitude -= preGainWindow[filterWindow - 1] * preGainWindow[filterWindow - 1] / (1 + bandCompress * preGainWindow[filterWindow - 1] * preGainWindow[filterWindow - 1]);
		double rootPre = sqrt(preAmplitude);
		splitValue *= gain + band * linearGain;
		splitValue = splitValue / (1 + abs(splitValue));
		if (splitValue > hardClip + band * linearHardClip) {
			splitValue = hardClip + band * linearHardClip;
		}
		if (splitValue < -hardClip - band * linearHardClip) {
			splitValue = -hardClip - band * linearHardClip;
		}
		postGainWindow.push_front(splitValue);
		postGainWindow.pop_back();
		postAmplitude += postGainWindow[0] * postGainWindow[0];
		postAmplitude -= postGainWindow[filterWindow - 1] * postGainWindow[filterWindow - 1];
		double rootPost = sqrt(postAmplitude);
		if (rootPost < 1) {
			rootPost = 1;
		}
		splitValue *= volumeFollow * rootPre / rootPost + 1 - volumeFollow;
		std::unique_lock<std::mutex> outputLock(outputMutex);
		processedValue += bandClip * (1 + (band - bands / 2.0) * linearMix) * splitValue / bands;
		outputLock.unlock();
		sync.arrive_and_wait();
	}
}

void Amplifier::processOutput(std::string File1, std::string File2, std::string File3, std::string File4, std::string File5, std::string File6, std::string outFile, int totalLength) {
	window.assign(filterWindow, 0);
	outputWindow.assign(filterWindow, 0);
	std::ifstream EStream(File1);
	std::ifstream AStream(File2);
	std::ifstream DStream(File3);
	std::ifstream GStream(File4);
	std::ifstream BStream(File5);
	std::ifstream eStream(File6);
	std::ofstream output(outFile);
	std::string comma;
	amplitude = 0;
	for (int i = 0; i < totalLength + 22050; i++) {
		value = 0;
		processedValue = 0;
		EStream >> stringAddition;
		value += stringAddition;
		EStream >> comma;
		AStream >> stringAddition;
		value += stringAddition;
		AStream >> comma;
		DStream >> stringAddition;
		value += stringAddition;
		DStream >> comma;
		GStream >> stringAddition;
		value += stringAddition;
		GStream >> comma;
		BStream >> stringAddition;
		value += stringAddition;
		BStream >> comma;
		eStream >> stringAddition;
		value += stringAddition;
		eStream >> comma;
		value = value;
		if (bandClip != 0) {
			window.push_front(value);
			window.pop_back();
			for (int j = 0; j < 200; j++) {
				window[0] -= (1 + sin(0.1 * j)) * feedbackPower * (outputWindow[j] / (1.0 + (200.0 - j) + abs(outputWindow[j]))) * (0.7 / (1 + 0.1 * (20 + feedbackWavelength - j) * (20 + feedbackWavelength - j)) + 1 / (1 + 0.1 * (feedbackWavelength - j) * (feedbackWavelength - j)));
			}
		}
		sync.arrive_and_wait();
		//Band threads run in between these two synchonizations
		sync.arrive_and_wait();
		std::unique_lock<std::mutex> outputLock(outputMutex);
		processedValue += (1 - bandClip) * value;
		if (i % 10000 == 0) {
			std::cout << i * timeStep << " seconds logged.\r\n";
		}
		if (compress != 0) {
			amplitude += outputWindow[0] * outputWindow[0];
			amplitude -= outputWindow[filterWindow - 1] * outputWindow[filterWindow - 1];
			double root = sqrt(amplitude);
			if (root < threshold) {
				root = threshold;
			}
			processedValue *= threshold * compress / root + 1 - compress;
		}
		if (processedValue > 1 / postGain) {
			processedValue = 1 / postGain;
		}
		if (processedValue < -1 / postGain) {
			processedValue = -1 / postGain;
		}
		outputWindow.push_front(0.9 * postGain * processedValue);
		outputWindow.pop_back();
		processedValue /= ((double)comb + 1);
		for (int k = 1; k < comb + 1; k++) {
			processedValue += (1.0 - 2.0 * (2 % k)) * outputWindow[k * k * combWidth] / ((double)comb + 1);
		}
		output << 0.9 * postGain * processedValue << " , ";
		outputLock.unlock();
	}
	output.close();
}