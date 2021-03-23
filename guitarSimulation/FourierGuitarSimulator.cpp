#include <iostream>
#include <thread>
#include <filesystem>
#include "FourierString.h"
#include "Amplifier.h"
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
		FourierString AString(180, 1, filePath);
		int totalLength = AString.totalMusicLength;
		FourierString BString(120, 4, filePath);
		FourierString DString(160, 2, filePath);
		FourierString highEString(100, 5, filePath);
		FourierString EString(200, 0, filePath);

		FourierString GString(140, 3, filePath);


	
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
		/*for (int i = 0; i < totalLength + 22050; i++) {
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
			output << processedValue << ", ";
		}
		output.close();*/
		Amplifier amp(outputDir + "EFile.txt", outputDir + "AFile.txt", outputDir + "DFile.txt", outputDir + "GFile.txt", outputDir + "BFile.txt", outputDir + "HighEFile.txt", outputDir + "output.txt", totalLength);
	}
	else
	{
		std::cout << filePath + " not found\n";
	}
}

