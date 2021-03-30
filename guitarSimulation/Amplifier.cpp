#include "Amplifier.h"

Amplifier::Amplifier(std::string File1, std::string File2, std::string File3, std::string File4, std::string File5, std::string File6, std::string outFile, int totalLength)
{
	double timeStep = 1 / (double)(44100);
	double bandClip = 0;
	double stringAddition;
	double value = 0;
	double processedValue = 0;
	double splitValue = 0;
	double compress = 0.8;
	double threshold = 1;
	double gain = 80;
	double linearGain = 0;
	double hardClip = 0.8;
	int bandSpread = 400;
	int bands = 4;
	int filterWindow = 100;
	int comb = 0;
	int combWidth = 4;
	double postGain = 0.5;
	double linearMix = -0.25;
	//double feedbackPower = 10;
	double feedbackPower = 0;
	double feedbackWavelength = 100;
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
				window[399] -= (1 + sin(0.1 * j)) * feedbackPower * (outputWindow[j] / (1 + 2 * j + abs(outputWindow[j]))) * (0.7 / (1 + 0.1 * (j - 399 + 20 + feedbackWavelength) * (j - 399 + 20 + feedbackWavelength)) + 1 / (1 + 0.1 * (j - 399 + feedbackWavelength) * (j - 399 + feedbackWavelength)));
			}
			for (int k = 0; k < bands; k++) {
				double splitValue = 0;
				splitValue += window[399];
				for (int j = 0; j < filterWindow; j++) {
					splitValue += window[399 - j] * sin(timeStep * (399 - j) * 100) * cos((500 + (bandSpread * k * k + 5000 * k)) * timeStep * j) / (399 - j);
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
		double amplitude = 0;
		for (int j = 0; j < 399; j++) {
			amplitude += outputWindow[j] * outputWindow[j] / (50 + j);
		}
		if (amplitude < threshold) {
			amplitude = threshold;
		}
		processedValue *= threshold * compress / amplitude + 1 - compress;
		if (processedValue > 1 / postGain) {
			processedValue = 1 / postGain;
		}
		if (processedValue < -1 / postGain) {
			processedValue = -1 / postGain;
		}
		outputWindow[399] = 0.9 * postGain * processedValue;
		processedValue /= (double)(comb + 1);
		for (int k = 1; k < comb + 1; k++) {
			processedValue += (1 - 2 * (2 % k)) * outputWindow[399 - k * k * combWidth] / (double)(comb + 1);
		}
		output << 0.9 * postGain * processedValue << " , ";
	}
	output.close();
}
