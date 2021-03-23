#include "Amplifier.h"

Amplifier::Amplifier(std::string File1, std::string File2, std::string File3, std::string File4, std::string File5, std::string File6, std::string outFile, int totalLength)
{
	double stringAddition;
	double value = 0;
	double processedValue = 0;
	double compress = 0.7;
	double threshold = 0.2;
	double gain = 0.1;
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
		value = 3 * value;
		window[399] = value;
		for (int i = 0; i < 399; i++) {
			window[399] -= outputWindow[i] / ( 1 + 0.1 * ( i - 200 ) * ( i - 200 ) );
		}
		double amplitude = 0;
		for (int j = 0; j < 399; j++) {
			amplitude += (window[j] - window[j+1]) * (window[j] - window[j + 1]) / (50 + j);
			window[j] = window[j + 1];
			outputWindow[j] = outputWindow[j + 1];
		}
		if (amplitude < threshold) {
			amplitude = threshold;
		}
		double compressed = (threshold * compress / amplitude + 1 - compress) * window[399];
		compressed *= gain;
		processedValue += compressed / (1 + abs(compressed));
		outputWindow[399] = processedValue;
		output << processedValue << " , ";
	}
	output.close();
}
