#include <iostream>
#include <thread>
#include <filesystem>
#include "FourierString.h"
int main(int argc, char** argv)
{
	std::string fileName;
	std::string outputDir;
	if (argc == 3)
	{
		fileName = argv[1];
		fileName = "guitarSimulation\\parsedSongFiles\\" + fileName;
		//outputDir = argv[2]+ "\" + argv[1];
	}
	else
	{
		std::cin >> fileName;
		fileName = "guitarSimulation\\parsedSongFiles\\" + fileName;
		outputDir = "";
	}
	//fileName = fileName + ".txt";
	if (std::filesystem::exists(fileName))
	{
		FourierString EString(200, 0, outputDir+fileName);
		FourierString AString(180, 1, outputDir + fileName);
		FourierString DString(160, 2, outputDir + fileName);
		FourierString GString(140, 3, outputDir + fileName);
		FourierString BString(120, 4, outputDir + fileName);
		FourierString highEString(100, 5, outputDir + fileName);
		int totalLength = AString.totalMusicLength;
		std::thread EThread(&FourierString::simulate, &EString, "EFile.txt");
		std::thread AThread(&FourierString::simulate, &AString, "AFile.txt");
		std::thread DThread(&FourierString::simulate, &DString, "DFile.txt");
		std::thread GThread(&FourierString::simulate, &GString, "GFile.txt");
		std::thread BThread(&FourierString::simulate, &BString, "BFile.txt");
		std::thread highEThread(&FourierString::simulate, &highEString, "HighEFile.txt");
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
		std::ifstream EStream("EFile.txt");
		std::ifstream AStream("AFile.txt");
		std::ifstream DStream("DFile.txt");
		std::ifstream GStream("GFile.txt");
		std::ifstream BStream("BFile.txt");
		std::ifstream eStream("HighEFile.txt");
		std::ofstream output("output.txt");
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
		std::cout << fileName + " not found\n";
	}
}

