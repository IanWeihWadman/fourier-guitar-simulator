#pragma once
#include <string>
#include <deque>
#include <mutex>
#include <semaphore>
#include <barrier>
class Amplifier
{
public:
	Amplifier(std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string, int);
private:
	void processBand(int band, int totalLength);
	void processOutput(std::string File1, std::string File2, std::string File3, std::string File4, std::string File5, std::string File6, std::string outFile, int totalLength);
	std::deque<double> window;
	std::deque<double> outputWindow;
	double bandClip = 0;
	double stringAddition = 0;
	double value = 0;
	double processedValue = 0;
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
	double lowestFreq = 0;
	double linearSpread = 0;
	double quadraticSpread = 0;
	int bands = 0;
	int filterWindow = 8000;
	int comb = 0;
	int combWidth = 0;
	double postGain = 0.8;
	double linearMix = 0;
	double feedbackPower = 0;
	double feedbackWavelength = 0;
	double timeStep = 0;
	std::barrier<> sync;
	std::mutex outputMutex;
};

