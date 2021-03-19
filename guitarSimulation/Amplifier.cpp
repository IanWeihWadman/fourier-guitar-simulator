#include "Amplifier.h"

Amplifier::Amplifier(std::string File1, std::string File2, std::string File3, std::string File4, std::string File5, std::string File6, int totalLength)
{
	std::ifstream EStream(File1);
	std::ifstream AStream(File2);
	std::ifstream DStream(File3);
	std::ifstream GStream(File4);
	std::ifstream BStream(File5);
	std::ifstream eStream(File6);
	std::ofstream output("output.txt");
	double stringAddition;
	double value = 0;
	double processedValue = 0;
	std::string comma;
	//This part just combines all the strings into the output file and applies soft clipping, but any extra effects you want to apply before outputting could go here
	for (int i = 0; i < totalLength + 22050; i++) {
		value = 0;
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
		processedValue = value;
		if (processedValue > 0) {
			processedValue = processedValue / (1 + processedValue);
		}
		if (processedValue < 0) {
			processedValue = processedValue / (1 - processedValue);
		}
		output << processedValue << ", ";
	}
	output.close();
}
