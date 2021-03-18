#pragma once
#include <vector>
#include <fstream>
#include <string>
#include <math.h>
class FourierString
{
	struct fret {
		fret(double inStart, double inEnd, int inFret, int inPull) :
			start{ inStart }, end{ inEnd }, fretPosition{ inFret }, pull{ inPull } {}
		double start;
		double end;
		int fretPosition;
		int pull;
	};
	struct mute {
		mute(double inStart, double inEnd, double inLocation, double inWidth, int inDamp) :
			start{ inStart }, end{ inEnd }, location{ inLocation }, damp{ inDamp }, width{ inWidth } {}
		double start;
		double end;
		double location;
		double width;
		int damp;
	};
	struct pick {
		pick(int inStart, int inDelay, double inForce) :
			start{ inStart }, delay{ inDelay }, force{ inForce } {}
		int start;
		int delay;
		double force;
	};
	struct bend {
		bend(int inStart, int inDuration, int inVibSpeed, std::vector<double> inVib, std::vector<double> inBend) :
			start{ inStart }, duration{ inDuration }, vibSpeed{ inVibSpeed }, vibPolys{ inVib }, bendPolys{ inBend } {}
		int start;
		int duration;
		int vibSpeed;
		std::vector<double> vibPolys;
		std::vector<double> bendPolys;
	};
public:
	FourierString(int tones, int number, std::string fileName);
	void simulate(std::string file);
	int totalMusicLength;
private:
	int overtones;
	int stringNumber;
	std::vector<double> State;
	std::vector<double> Derivative;
	std::vector<double> Statek1;
	std::vector<double> Derivativek1;
	std::vector<double> Statek2;
	std::vector<double> Derivativek2;
	std::vector<double> Statek3;
	std::vector<double> Derivativek3;
	std::vector<double> Statek4;
	std::vector<double> Derivativek4;
	std::vector<double> pickResponse;
	std::vector<double> pickDisruption;
	std::vector<double> resonances;
	std::vector<double> resonanceDerivatives;
	std::vector<double> resonanceFrequencies;
	std::vector<double> resonanceDamping;
	std::vector<double> tensionModifiers;
	std::vector<std::vector<std::vector<std::vector<double>>>> fretChangeMatrix;
	std::vector<std::vector<double>> resonateCoupling;
	std::vector<pick> picking;
	std::vector<fret> fretting;
	std::vector<mute> muting;
	std::vector<bend> bending;
	double pulloffForce;
	double highFretDamping;
	double quadraticDamping;
	double linearDamping;
	double constantDamping;
	double overallNonlinearity;
	double highFrequencyNonlinearity;
	double constantPickDisruption;
	double linearPickDisruption;
	double constantPulloffDisruption;
	double linearPulloffDisruption;
	double pickingWidth;
	double pickingLocation;
	double pickHardness;
	double pickScratch;
	double linearMuting;
	double pickupWidth;
	double pickupLocation;
	double stringBrightness;
	double pickupValue;
	int resonanceNum;
	double currentPickForce;
	double oldPickForce;
	double newPickForce;
	double frequency;
	int pickDirection;
	double oldTensionMod;
	double newTensionMod;
	double currentTensionMod;
	double tension;
	int currentFret;
	int oldFret;
	double timeStep;
	int stepsPerSample;
	double tempo;
	int bars;
	int subdivision;
	int updatesPerSubdivision;
	std::vector<double> naturalDamping;
	std::vector<double> addedDamping;
	double nextParamUpdate;
	void parseMusicFiles(std::string fileName);
	double updateState();
	void updateParameters(double weight);
	int getStepFromMeasureTime(double time);
	double getMeasureTimeFromStep(int step);
	void computeNewParameters(int currentStep);
	std::vector<double> matrixMultiply(const std::vector<std::vector<double>>& matrix, const std::vector<double>& input);
	double spline(const std::vector<double>& points, double input);
	double pickupResponse(double fretScale, int overtone);
};

