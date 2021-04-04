#include <iostream>
#include <thread>
#include <filesystem>
#include "FourierString.h"
#include "Amplifier.h"
#include "ketopt.h"

namespace fs = std::filesystem;

int main(int argc, char** argv)
{
	std::string songName;
	std::string fileNameString;
	std::string ampPresetOverride;
	std::string outputDir;

	bool no_sim = false;
	fileNameString = "";

	// options:  
	//		s <name of tab file>			- optional, requested as console input if not available, assumed to be in ".\parseOut\"
	//		a <amplifier preset override>	- optional, overrides amp preset from tab
	//		n								- optional, skips simulation and only applies amplifier
	int opt;
	ketopt_t optarg;
	optarg = KETOPT_INIT;
	while ((opt = ketopt(&optarg, argc, argv, false,"a:s:n", NULL )) != -1) {
		switch (opt) {
		case 'n':
			no_sim = true;
			break;
		case 's':
			fileNameString = optarg.arg;
			break;
		case 'a':
			ampPresetOverride = optarg.arg;
			break;

		}
	}

	if (fileNameString.length() == 0)
	{
		std::cin >> fileNameString;
	}
	fs::path filePath(fileNameString);
	songName = filePath.filename().stem().string();
	outputDir = "simOut\\" + songName + "\\";
	std::string inputPathString = "parseOut\\" + songName + ".out";
	
	std::cout << "CWD: " + fs::current_path().string() +"\n";
	fs::create_directory(outputDir);
	if (fs::exists(inputPathString))
	{
		FourierString AString(180, 1, inputPathString); // need this one to read song info, if that's removed or accessed differently this could be skipped as well
		int totalLength = AString.totalMusicLength;
		if (!no_sim)
		{
			FourierString AString(180, 1, inputPathString); // put here for hacking 

			/*FourierString BString(120, 4, inputPathString);
			FourierString DString(160, 2, inputPathString);
			FourierString highEString(100, 5, inputPathString);
			FourierString EString(200, 0, inputPathString);
			FourierString GString(140, 3, inputPathString);
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
			double processedValue = 0;*/
		}

		std::string ampPreset = (ampPresetOverride.length() > 0) ? ampPresetOverride : AString.getAmpPreset();

		Amplifier amp(outputDir + "EFile.txt", outputDir + "AFile.txt", outputDir + "DFile.txt", outputDir + "GFile.txt", outputDir + "BFile.txt", outputDir + "HighEFile.txt", outputDir + "output.txt", ampPreset, totalLength);
	}
	else
	{
		std::cout << inputPathString + " not found\n";
	}
}

