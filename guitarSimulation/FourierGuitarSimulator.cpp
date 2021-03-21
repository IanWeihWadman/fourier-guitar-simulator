#include <iostream>
#include <thread>
#include <filesystem>
#include "FourierString.h"
int main(int argc, char** argv)
{
	std::string filePath;
	std::string fileName;
	std::string outputDir;
	if (argc == 2)
	{
		fileName = argv[1];
	}
	else
	{
		std::cin >> fileName;

	}
	filePath = "parseOut\\" + fileName + ".out";
	outputDir = "simOut\\" + fileName + "\\";
	std::filesystem::path outputPath(outputDir);
	std::cout << "CWD: " + std::filesystem::current_path().string(); +"\n";
	std::filesystem::create_directory(outputDir);
	if (std::filesystem::exists(filePath))
	{
		FourierString EString(200, 0, outputDir + filePath);
		FourierString AString(180, 1, outputDir + filePath);
		FourierString DString(160, 2, outputDir + filePath);
		FourierString GString(140, 3, outputDir + filePath);
		FourierString BString(120, 4, outputDir + filePath);
		FourierString highEString(100, 5, outputDir + filePath);
		int totalLength = AString.totalMusicLength;
		std::thread EThread(&FourierString::simulate, &EString, outputDir + "EFile.txt");
		std::thread AThread(&FourierString::simulate, &AString, outputDir + "AFile.txt");
		std::thread DThread(&FourierString::simulate, &DString, outputDir + "DFile.txt");
		std::thread GThread(&FourierString::simulate, &GString, outputDir + "GFile.txt");
		std::thread BThread(&FourierString::simulate, &BString, outputDir + "BFile.txt");
		std::thread highEThread(&FourierString::simulate, &highEString, outputDir + "HighEFile.txt");
		EThread.join();
		AThread.join();
		DThread.join();
		GThread.join();
		BThread.join();
		highEThread.join();
		double value = 0;
		double stringAddition = 0;
		double processedValue = 0;
		std::string comma;
		std::ifstream EStream(outputDir + "EFile.txt");
		std::ifstream AStream(outputDir + "AFile.txt");
		std::ifstream DStream(outputDir + "DFile.txt");
		std::ifstream GStream(outputDir + "GFile.txt");
		std::ifstream BStream(outputDir + "BFile.txt");
		std::ifstream eStream(outputDir + "HighEFile.txt");
		std::ofstream output("simOut\\" + filePath + ".simout.txt");
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
			processedValue = 0.6 * value;
			if (processedValue > 1.8) {
				processedValue = 1.8;
			}
			if (processedValue < -1.8) {
				processedValue = -1.8;
			}
			processedValue = processedValue / (1 + 0.3 * processedValue * processedValue);
			/*processedValue = 0.25 * value;
			if (processedValue > 1) {
				std::cout << "clipped \r\n";
				processedValue = 1;
			}
			if (processedValue < -1) {
				std::cout << "clipped \r\n";
				processedValue = -1;
			}*/
			output << processedValue << ", ";
		}
		output.close();
	}
	else
	{
		std::cout << filePath + " not found\n";
	}
}

