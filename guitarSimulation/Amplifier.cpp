#include "Amplifier.h"

Amplifier::Amplifier(std::string File1, std::string File2, std::string File3, std::string File4, std::string File5, std::string File6, std::string outFile, int totalLength)
{
	double stringAddition;
	double value = 0;
	double processedValue = 0;
	double compress = 0.8;
	double threshold = 0.1;
	double gain = 0.3;
	double hardClip = 0.16;
	//double feedbackPower = 48;
	double feedbackPower = 0;
	double feedbackWavelength = 150;
	std::ifstream EStream(File1);
	std::ifstream AStream(File2);
	std::ifstream DStream(File3);
	std::ifstream GStream(File4);
	std::ifstream BStream(File5);
	std::ifstream eStream(File6);
	std::ofstream output(outFile);
	std::string comma;
	std::vector<double> window;
	std::vector<double> outputWindow;
	window.assign(400, 0);
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
		value = 12 * value;
		window[399] = value;
		for (int i = 0; i < 399; i++) {
			window[399] -= (1 + sin(0.1 * i)) * feedbackPower * (outputWindow[i] / (1 + 0.6 * i + abs(outputWindow[i]))) * ( 0.7 / ( 1 + 0.1 * ( i - 399 + 50 + feedbackWavelength ) * ( i - 399 + 50 + feedbackWavelength )) + 1 / ( 1 +  0.1 * (i - 399 + feedbackWavelength) * (i - 399 + feedbackWavelength)));
		}
		double amplitude = 0;
		for (int j = 0; j < 399; j++) {
			amplitude += window[j] * window[j] / (50 + j);
			window[j] = window[j + 1];
			outputWindow[j] = outputWindow[j + 1];
		}
		if (amplitude < threshold) {
			amplitude = threshold;
		}
		double compressed = (threshold * compress / amplitude + 1 - compress) * window[399];
		compressed *= gain;
		processedValue += compressed / (1 + abs(compressed));
		if (processedValue > hardClip) {
			processedValue = hardClip;
		}
		if (processedValue < -hardClip) {
			processedValue = -hardClip;
		}
		outputWindow[399] = processedValue;
		output << processedValue << " , ";
	}
	output.close();
}
