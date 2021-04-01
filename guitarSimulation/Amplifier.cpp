#include "Amplifier.h"

Amplifier::Amplifier(std::string File1, std::string File2, std::string File3, std::string File4, std::string File5, std::string File6, std::string outFile, std::string presetFile, int totalLength)
{
	double timeStep = 1 / (double)(44100);
	double bandClip = 0;
	double stringAddition;
	double value = 0;
	double processedValue = 0;
	double preAmplitude = 0;
	double postAmplitude = 0;
	double amplitude = 0;
	double splitValue = 0;
	double compress = 0;
	double threshold = 0;
	double gain = 0;
	double linearGain = 0;
	double hardClip = 0;
	double linearHardClip = 0;
	double volumeFollow = 0;
	double bandCompress = 0;
	int lowestFreq = 0;
	int linearSpread = 0;
	int quadraticSpread = 0;
	int bands = 0;
	int filterWindow = 8000;
	int comb = 0;
	int combWidth = 0;
	double postGain = 0.8;
	double linearMix = 0;
	double feedbackPower = 0;
	double feedbackWavelength = 0;
	std::ifstream presetStream("guitarSimulation\\amppresets\\" + presetFile + ".txt");
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
	std::ifstream EStream(File1);
	std::ifstream AStream(File2);
	std::ifstream DStream(File3);
	std::ifstream GStream(File4);
	std::ifstream BStream(File5);
	std::ifstream eStream(File6);
	std::ofstream output(outFile);
	std::string comma;
	std::deque<double> window;
	window.assign(filterWindow, 0);
	std::deque<double> outputWindow;
	outputWindow.assign(filterWindow, 0);
	std::vector<std::deque<double>> preWindow;
	std::vector<std::deque<double>> postWindow;
	std::vector<std::vector<double>> filterProfiles;
	for (int i = 0; i < bands; i++) {
		std::deque<double> nextVec;
		nextVec.assign(filterWindow, 0);
		preWindow.push_back(nextVec);
	}
	for (int i = 0; i < bands; i++) {
		std::deque<double> nextVec;
		nextVec.assign(filterWindow, 0);
		postWindow.push_back(nextVec);
	}
	for (int i = 0; i < bands; i++) {
		std::vector<double> nextVec;
		filterProfiles.push_back(nextVec);
	}
	for (int k = 0; k < bands; k++) {
		filterProfiles[k].push_back(1);
		for (int j = 1; j < 1000; j++) {
			filterProfiles[k].push_back(sin(timeStep * j * 400 / (1 + (double)bands)) * cos((lowestFreq + ((double)quadraticSpread * k * k + (double)linearSpread * k)) * timeStep * j) / j);
		}
	}
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
			for (int k = 0; k < bands; k++) {
				double splitValue = 0;
				for (int j = 0; j < 1000; j++) {
					splitValue += window[j] * filterProfiles[k][j];
				}
				preWindow[k].push_front(splitValue);
				preWindow[k].pop_back();
				preAmplitude += preWindow[k][0] * preWindow[k][0] / (1 + bandCompress * preWindow[k][0] * preWindow[k][0]);
				preAmplitude -= preWindow[k][filterWindow - 1] * preWindow[k][filterWindow - 1] / (1 + bandCompress * preWindow[k][filterWindow - 1] * preWindow[k][filterWindow - 1]);
				double rootPre = sqrt(preAmplitude);
				splitValue *= gain + k * linearGain;
				splitValue = splitValue / (1 + abs(splitValue));
				if (splitValue > hardClip + k * linearHardClip) {
					splitValue = hardClip + k * linearHardClip;
				}
				if (splitValue < -hardClip - k * linearHardClip) {
					splitValue = -hardClip - k * linearHardClip;
				}
				postWindow[k].push_front(splitValue);
				postWindow[k].pop_back();
				postAmplitude += postWindow[k][0] * postWindow[k][0];
				postAmplitude -= postWindow[k][filterWindow - 1] * postWindow[k][filterWindow - 1];
				double rootPost = sqrt(postAmplitude);
				if (rootPost < 1) {
					rootPost = 1;
				}
				splitValue *= volumeFollow * rootPre / rootPost + 1 - volumeFollow;
				processedValue += bandClip * (1 + (k - bands / 2.0) * linearMix) * splitValue / bands;
			}
		}
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
		processedValue /= ((double) comb + 1);
		for (int k = 1; k < comb + 1; k++) {
			processedValue += (1.0 - 2.0 * (2 % k)) * outputWindow[k * k * combWidth] / ((double) comb + 1);
		}
		output << 0.9 * postGain * processedValue << " , ";
	}
	output.close();
}
