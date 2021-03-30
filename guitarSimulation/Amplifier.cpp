#include "Amplifier.h"

Amplifier::Amplifier(std::string File1, std::string File2, std::string File3, std::string File4, std::string File5, std::string File6, std::string outFile, std::string presetFile, int totalLength)
{
	double timeStep = 1 / (double)(44100);
	double bandClip = 0.8;
	double stringAddition;
	double value = 0;
	double processedValue = 0;
	double splitValue = 0;
	double compress = 0.5;
	double threshold = 1;
	double gain = 1.2;
	double linearGain = 0.2;
	double hardClip = 1;
	int lowestFreq = 400;
	int linearSpread = 3000;
	int quadraticSpread = 450;
	int bands = 5;
	int filterWindow = 100;
	int comb = 5;
	int combWidth = 5;
	double postGain = 1.2;
	double linearMix = 0;
	double feedbackPower = 1;
	double feedbackWavelength = 70;
	std::ifstream presetStream("guitarSimulation\\amppresets\\" + presetFile + ".txt");
	presetStream >> bandClip;
	presetStream >> compress;
	presetStream >> threshold;
	presetStream >> gain;
	presetStream >> linearGain;
	presetStream >> hardClip;
	presetStream >> lowestFreq;
	presetStream >> linearSpread;
	presetStream >> quadraticSpread;
	presetStream >> bands;
	presetStream >> filterWindow;
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
	std::vector<double> window;
	window.assign(400, 0);
	std::vector<double> outputWindow;
	outputWindow.assign(400, 0);
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
		for (int j = 0; j < 399; j++) {
			window[j] = window[j + 1];
			outputWindow[j] = outputWindow[j + 1];
		}
		if (bandClip != 0) {
			window[399] = value;
			for (int j = 0; j < 399; j++) {
				window[399] -= (1 + sin(0.1 * j)) * feedbackPower * (outputWindow[j] / (1.0 + 2.0 * j + abs(outputWindow[j]))) * (0.7 / (1 + 0.1 * (j - 399.0 + 20 + feedbackWavelength) * (j - 399.0 + 20 + feedbackWavelength)) + 1 / (1 + 0.1 * (j - 399.0 + feedbackWavelength) * (j - 399.0 + feedbackWavelength)));
			}
			for (int k = 0; k < bands; k++) {
				double splitValue = 0;
				splitValue += window[399];
				for (int j = 0; j < filterWindow; j++) {
					splitValue += window[399 - j] * sin(timeStep * (399.0 - j) * 400 / (1 + bands)) * cos((lowestFreq + ((double) quadraticSpread * k * k + linearSpread * k)) * timeStep * j) / (399.0 - j);
				}
				splitValue *= gain + k * linearGain;
				splitValue = splitValue / (1 + abs(splitValue));
				if (splitValue > hardClip) {
					splitValue = hardClip;
				}
				if (splitValue < -hardClip) {
					splitValue = -hardClip;
				}
				processedValue += bandClip * (1 + (k - bands / 2.0) * linearMix) * splitValue / bands;
			}
		}
		processedValue += (1 - bandClip) * value;
		if (i % 10000 == 0) {
			std::cout << i * timeStep << " seconds logged.\r\n";
		}
		if (compress != 0) {
			double amplitude = 0;
			for (int j = 0; j < 399; j++) {
				amplitude += outputWindow[j] * outputWindow[j] / (50.0 + j);
			}
			if (amplitude < threshold) {
				amplitude = threshold;
			}
			processedValue *= threshold * compress / amplitude + 1 - compress;
		}
		if (processedValue > 1 / postGain) {
			processedValue = 1 / postGain;
		}
		if (processedValue < -1 / postGain) {
			processedValue = -1 / postGain;
		}
		outputWindow[399] = 0.9 * postGain * processedValue;
		processedValue /= ((double) comb + 1);
		for (int k = 1; k < comb + 1; k++) {
			processedValue += (1.0 - 2.0 * (2 % k)) * outputWindow[399 - k * k * combWidth] / ((double) comb + 1);
		}
		output << 0.9 * postGain * processedValue << " , ";
	}
	output.close();
}
