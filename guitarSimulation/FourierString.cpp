#include "FourierString.h"
#include <iostream>

FourierString::FourierString(int tones, int number, std::string fileName)
{
	oldFret = 0;
	currentFret = 0;
	overtones = tones;
	stringNumber = number;
	//Default values, usually overwritten when parseMusicFiles is called
	int fretCount = 20;
	bars = 1;
	tempo = 1;
	subdivision = 1;
	updatesPerSubdivision = 1;
	highFretDamping = 0.05;
	quadraticDamping = 0.1;
	linearDamping = 0.1;
	constantDamping = 3;
	overallNonlinearity = 0.004;
	highFrequencyNonlinearity = 0.002;
	pickingWidth = 30;
	pickHardness = 0.2;
	pickScratch = 0;
	pickingLocation = 0.151;
	linearMuting = 0.1;
	pickupWidth = 0.25;
	pickupLocation = 0.163;
	stringBrightness = 0.1;
	resonanceNum = 0;
	tensionDecrease = 0.45;
	acoustic = 0;
	stepsPerSample = 16;
	if (stringNumber == 0) {
		frequency = 82.4;
	}
	if (stringNumber == 1) {
		frequency = 110;
	}
	if (stringNumber == 2) {
		frequency = 146.8;
	}
	if (stringNumber == 3) {
		frequency = 196;
	}
	if (stringNumber == 4) {
		frequency = 246.94;
	}
	if (stringNumber == 5) {
		frequency = 329.6;
	}
	//I don't know where 40 comes from, but it seems to be a magic number that gets the correct tuning. Probably not exactly right but I will come back later
	tension = 40 * frequency * frequency;
	parseMusicFiles(fileName);
	totalMusicLength = 44100 * bars / tempo + 10000;
	timeStep = 1 / (double) (44100 * stepsPerSample);
	pickDisruption.assign(overtones, 0);
	State.assign(overtones, 0);
	Derivative.assign(overtones, 0);
	Statek1.assign(overtones, 0);
	Derivativek1.assign(overtones, 0);
	Statek2.assign(overtones, 0);
	Derivativek2.assign(overtones, 0);
	Statek3.assign(overtones, 0);
	Derivativek3.assign(overtones, 0);
	Statek4.assign(overtones, 0);
	Derivativek4.assign(overtones, 0);
	addedDamping.assign(overtones, 0);
	resonances.assign(resonanceNum, 0);
	resonanceDerivatives.assign(resonanceNum, 0);
	resonanceDamping.assign(resonanceNum, 0);
	resonanceFrequencies.assign(resonanceNum, 0);
	tensionModifiers.assign(overtones, 0);
	naturalDamping.assign(overtones, 0);
	newTensionMod = 0;
	newPickForce = 0;
	oldPickForce = 0;
	oldTensionMod = 0;
	pulloffForce = 0;
	currentPickForce = 0;
	currentTensionMod = 0;
	for (int i = 0; i < overtones; i++) {
		tensionModifiers[i] = (i + 1) * (i + 1) * ( 1 - tensionDecrease * i * i / (double) (overtones * overtones));
	}
	for (int i = 0; i < resonanceNum; i++) {
		resonanceDamping[i] = 3 - i * 2.5 / (double) resonanceNum;
		resonanceFrequencies[i] = 30000 + 6000 * i + 700 * sqrt(i);
	}
	//Every pair of frets has a matrix associated with the transition between those two frets representing a linear transformation on the Fourier coefficients of the state
	for (int p = 0; p < fretCount; p++) {
		std::vector<std::vector<std::vector<double>>> nextMatrixSet;
		double oldFretScale = pow(2, p / (double) 12);
		for (int k = 0; k < fretCount; k++) {
			std::vector<std::vector<double>> nextMatrix;
			double newFretScale = pow(2, k / (double)12);
			for (int i = 0; i < overtones; i++) {
				std::vector<double> nextVec;
				nextVec.assign(overtones, 0);
				nextMatrix.push_back(nextVec);
				//Each matrix starts out identical. These are not the mathematically correct values for this transition matrix, but they are simpler and I found it makes no difference to the sound
				for (int j = 0; j < overtones; j++) {
					nextMatrix[i][j] = 1 / (double) ( 1 + (i - j) * (i - j));
				}
			}
			//This section rescales the transition matrices to guarantee that the pickup response is the same before and after the fret change, avoiding clicking sounds
			for (int i = 0; i < overtones; i++) {
				double total = 0;
				std::vector<double> testVec;
				testVec.assign(overtones, 0);
				testVec[i] = 1;
				testVec = matrixMultiply(nextMatrix, testVec);
				for (int j = 0; j < overtones; j++) {
					total += pickupResponse(oldFretScale, j) * testVec[j];
				}
				for (int j = 0; j < overtones; j++) {
					nextMatrix[j][i] *= pickupResponse(newFretScale, i) / total;
				}
			}
			nextMatrixSet.push_back(nextMatrix);
		}
		fretChangeMatrix.push_back(nextMatrixSet);
	}
}

void FourierString::parseMusicFiles(std::string fileName)
{
	std::ifstream inStream(fileName);
	while (true) {
		char data;
		inStream >> data;
		if (data == '#') { break; }
		if (data == '{') {
			std::string dataType = "";
			while (true) {
				char nextChar;
				inStream >> nextChar;
				if (nextChar != ' ') {
					if (nextChar == '=' || nextChar == ':') {
						break;
					}
					else {
						dataType = dataType.append(1, nextChar);
					}
				}
			}
			if (dataType == "preset") {
				char nextChar = 'a';
				std::string preset = "";
				while (true)
				{
					inStream >> nextChar;
					if (nextChar != ' ') {
						if (nextChar == '}') {
							break;
						}
						else {
							preset = preset.append(1, nextChar);
						}
					}
				}
				std::ifstream presetStream("guitarSimulation\\presets\\" + preset + ".txt");
				double presetFreq;
				presetStream >> highFretDamping;
				presetStream >> quadraticDamping;
				presetStream >> linearDamping;
				presetStream >> constantDamping;
				presetStream >> overallNonlinearity;
				presetStream >> highFrequencyNonlinearity;
				presetStream >> pickingWidth;
				presetStream >> pickHardness;
				presetStream >> pickScratch;
				presetStream >> pickingLocation;
				presetStream >> linearMuting;
				presetStream >> pickupWidth;
				presetStream >> pickupLocation;
				presetStream >> stringBrightness;
				presetStream >> resonanceNum;
				presetStream >> tensionDecrease;
				presetStream >> acoustic;
				presetStream >> presetFreq;
				if (stringNumber == 0) {
					tension = 40 * presetFreq * presetFreq;
				}
				presetStream >> presetFreq;
				if (stringNumber == 1) {
					tension = 40 * presetFreq * presetFreq;
				}
				presetStream >> presetFreq;
				if (stringNumber == 2) {
					tension = 40 * presetFreq * presetFreq;
				}
				presetStream >> presetFreq;
				if (stringNumber == 3) {
					tension = 40 * presetFreq * presetFreq;
				}
				presetStream >> presetFreq;
				if (stringNumber == 4) {
					tension = 40 * presetFreq * presetFreq;
				}
				presetStream >> presetFreq;
				if (stringNumber == 5) {
					tension = 40 * presetFreq * presetFreq;
				}
			}
			if (dataType == "bars") {
				inStream >> bars;
			}
			if (dataType == "tempo") {
				inStream >> tempo;
			}
			if (dataType == "subdivisions") {
				inStream >> subdivision;
			}
			if (dataType == "updatefrequency") {
				inStream >> updatesPerSubdivision;
			}
			if (dataType == "highfretdamping") {
				inStream >> highFretDamping;
			}
			if (dataType == "quadraticdamping") {
				inStream >> quadraticDamping;
			}
			if (dataType == "lineardamping") {
				inStream >> linearDamping;
			}
			if (dataType == "constantdamping") {
				inStream >> constantDamping;
			}
			if (dataType == "overallnonlinearity") {
				inStream >> overallNonlinearity;
			}
			if (dataType == "highfrequencynonlinearity") {
				inStream >> highFrequencyNonlinearity;
			}
			if (dataType == "pickingwidth") {
				inStream >> pickingWidth;
			}
			if (dataType == "pickhardness") {
				inStream >> pickHardness;
			}
			if (dataType == "pickscratch") {
				inStream >> pickScratch;
			}
			if (dataType == "pickinglocation") {
				inStream >> pickingLocation;
			}
			if (dataType == "linearmuting") {
				inStream >> linearMuting;
			}
			if (dataType == "pickupwidth") {
				inStream >> pickupWidth;
			}
			if (dataType == "pickuplocation") {
				inStream >> pickupLocation;
			}
			if (dataType == "stringbrightness") {
				inStream >> stringBrightness;
			}
			if (dataType == "resonancenumber") {
				inStream >> resonanceNum;
			}
			if (dataType == "tensiondecrease") {
				inStream >> tensionDecrease;
			}
			if (dataType == "acoustic") {
				inStream >> acoustic;
			}
			if (dataType == "string0freq") {
				double freq;
				inStream >> freq;
				if (stringNumber == 0) {
					tension = 40 * freq * freq;
				}
			}
			if (dataType == "string1freq") {
				double freq;
				inStream >> freq;
				if (stringNumber == 1) {
					tension = 40 * freq * freq;
				}
			}
			if (dataType == "string2freq") {
				double freq;
				inStream >> freq;
				if (stringNumber == 2) {
					tension = 40 * freq * freq;
				}
			}
			if (dataType == "string3freq") {
				double freq;
				inStream >> freq;
				if (stringNumber == 3) {
					tension = 40 * freq * freq;
				}
			}
			if (dataType == "string4freq") {
				double freq;
				inStream >> freq;
				if (stringNumber == 4) {
					tension = 40 * freq * freq;
				}
			}
			if (dataType == "string5freq") {
				double freq;
				inStream >> freq;
				if (stringNumber == 5) {
					tension = 40 * freq * freq;
				}
			}
			if (dataType == "picking") {

				int start;
				int delay;
				int string;
				double force;
				char equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> start;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> delay;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> force;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> string;
				if (string == stringNumber) {
					picking.push_back(pick(start, delay, force));
				}
			}
			if (dataType == "fretting") {
				double start;
				double end;
				int string;
				int change;
				int pull;
				char equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> start;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> end;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> change;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> string;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> pull;
				if (string == stringNumber) {
					fretting.push_back(fret(start, end, change, pull));
				}
			}
			if (dataType == "muting") {
				double start;
				double end;
				int string;
				int force;
				double width;
				double location;
				char equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> start;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> end;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> force;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> string;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> width;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> location;
				if (string == stringNumber) {
					muting.push_back(mute(start, end, location, width, force));
				}
			}
			if (dataType == "bending") {
				int start;
				int duration;
				int string;
				double vibrato;
				double location;
				double vibspeed;
				char equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> start;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> duration;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> string;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> vibrato;
				equal = 0;
				while (equal != '=') {
					inStream >> equal;
				}
				inStream >> vibspeed;
				equal = 0;
				while (equal != '[') {
					inStream >> equal;
				}
				std::vector<double> inVib;
				inVib.push_back(0);
				for (int i = 0; i < duration - 1; i++) {
					inVib.push_back(vibrato);
				}
				inVib.push_back(0);
				std::vector<double> inBend;
				for (int i = 0; i < duration + 1; i++) {
					double bendValue;
					inStream >> bendValue;
					inStream >> equal;
					inBend.push_back(bendValue);
				}
				if (string == stringNumber) {
					bending.push_back(bend(start, duration, vibspeed, inVib, inBend));
				}
			}
		}
	}
	currentTensionMod = 0;
	oldTensionMod = 0;
	newTensionMod = 0;
	inStream.close();
}

double FourierString::updateState()
{
	//First handle fret changes
	if (currentFret != oldFret) {
		State = matrixMultiply(fretChangeMatrix[oldFret][currentFret], State);
		oldFret = currentFret;
	}
	pickupValue = 0;
	//Effective tension accounts for fretting, vibrato, bending
	double effectiveTension = (tension + currentTensionMod) * pow(2, (double)currentFret / 6);
	//This scaling factor accounts for the change in the effective "length" of the string caused by fretting
	double fretScale = pow(2, currentFret / (double)12);
	//Main calculation follows standard RK4 method
	for (int k = 0; k < stepsPerSample; k++) 
	{
		for (int i = 0; i < overtones; i++) {
			Statek1[i] = Derivative[i];
			Derivativek1[i] = -State[i] * tensionModifiers[i] * effectiveTension / (1 + overallNonlinearity * (highFrequencyNonlinearity * i + 1) * abs(State[i])) - (naturalDamping[i] + addedDamping[i]) * Derivative[i];
		}
		for (int i = 0; i < overtones; i++) {
			State[i] += 0.5 * timeStep * Statek1[i];
			Derivative[i] += 0.5 * timeStep * Derivativek1[i];
		}
		for (int i = 0; i < overtones; i++) {
			Statek2[i] = Derivative[i];
			Derivativek2[i] = -State[i] * tensionModifiers[i] * effectiveTension / (1 + overallNonlinearity * (highFrequencyNonlinearity * i + 1) * abs(State[i])) - (naturalDamping[i] + addedDamping[i]) * Derivative[i];
		}
		for (int i = 0; i < overtones; i++) {
			State[i] += 0.5 * timeStep * (Statek2[i] - Statek1[i]);
			Derivative[i] += 0.5 * timeStep * (Derivativek2[i] - Derivativek1[i]);
		}
		for (int i = 0; i < overtones; i++) {
			Statek3[i] = Derivative[i];
			Derivativek3[i] = -State[i] * tensionModifiers[i] * effectiveTension / (1 + overallNonlinearity * (highFrequencyNonlinearity * i + 1) * abs(State[i])) - (naturalDamping[i] + addedDamping[i]) * Derivative[i];
		}
		for (int i = 0; i < overtones; i++) {
			State[i] += 0.5 * timeStep * (2 * Statek3[i] - Statek2[i]);
			Derivative[i] += 0.5 * timeStep * (2 * Derivativek3[i] - Derivativek2[i]);
		}
		for (int i = 0; i < overtones; i++) {
			Statek4[i] = Derivative[i];
			Derivativek4[i] = -State[i] * tensionModifiers[i] * effectiveTension / (1 + overallNonlinearity * (highFrequencyNonlinearity * i + 1) * abs(State[i])) - (naturalDamping[i] + addedDamping[i]) * Derivative[i];
		}
		for (int i = 0; i < overtones; i++) {
			State[i] += 0.1666667 * timeStep * (Statek1[i] + 2 * Statek2[i] - 4 * Statek3[i] + Statek4[i]);
			Derivative[i] += 0.1666667 * timeStep * (Derivativek1[i] + 2 * Derivativek2[i] - 4 * Derivativek3[i] + Derivativek4[i]);
		}
		//Picking is described by a linear constraint, or more generally a set of linear constraints
		//If the inner product of the state with the constraint vector is below the target value, currentPickForce, a large force is immediately applied
		//in the direction of the constraint vector until the inner product is above the target value
		for (int q = 0; q < 8; q++) {
			if (currentPickForce != 0) {
				double pickSpot = 0;
				for (int i = 0; i < overtones; i++) {
					pickSpot += sin((pickingLocation + 0.003 * q) * 3.1415 * (i + 1) * fretScale) * State[i] * (pickingWidth + pickHardness * i + pickScratch * i * i);
				}
				if (pickSpot < currentPickForce) {
					for (int i = 0; i < overtones; i++) {
						//Force is boosted somewhat when tension is high to overcome the resistance due to string tension
						Derivative[i] += sin( (pickingLocation + 0.003 * q) * fretScale * 3.1415 * (i + 1)) * (500000 + tension + effectiveTension) * (pickingWidth + pickHardness * i + pickScratch * i * i) / 800000000;
					}
				}
			}
		}
		//Pull-offs are described by a milder force proportional to the distance between the state and a target value
		if (pulloffForce != 0) {
			for (int i = 0; i < overtones; i++) {
				Derivative[i] += 0.1 * (500000 + tension + effectiveTension) * (pulloffForce - State[i]) * (20 + i) / 800000000;
			}
		}
	}
	//Weighted sum of state vector produces the final value
	for (int i = 0; i < overtones; i++) {
		pickupValue += pickupResponse(fretScale, i) * State[i];
	}
	return pickupValue;
}

void FourierString::updateParameters(double weight)
{
	//This function interpolates between parameter values during steps when computeNewParameters is not called
	//These two parameters are the only ones sensitive enough to really need this treatment
	currentTensionMod = (1 - weight) * oldTensionMod + weight * newTensionMod;
	currentPickForce = (1 - weight) * oldPickForce + weight * newPickForce;
}

int FourierString::getStepFromMeasureTime(double time) {
	//Conversion between time in musical steps and time in 44.1kHz samples
	return (int)round(time * 44100 / (tempo * subdivision));
}

double FourierString::getMeasureTimeFromStep(int step)
{
	//Conversion between time in musical steps and time in 44.1kHz samples
	return (tempo * subdivision) * (double)step / 44100;
}

void FourierString::simulate(std::string file)
{
	//This function manages the overall simulation of one string and writes the results to the file passed as parameter
	double time = 0;
	double output = 0;
	double weight = 0;
	double nextParamUpdate = 0;
	std::ofstream outFile(file);
	double newStep = 0;
	if (picking.size() != 0) {
		for (int currentStep = 0; currentStep < totalMusicLength + 22050; currentStep++) {
			newStep = 0;
			time = getMeasureTimeFromStep(currentStep);
			if (time >= nextParamUpdate) {
				nextParamUpdate += 1 / (double)updatesPerSubdivision;
				computeNewParameters(currentStep);
			}
			weight = (double)updatesPerSubdivision * (time - nextParamUpdate + 1 / (double)updatesPerSubdivision);
			updateParameters(weight);
			newStep = updateState();
			outFile << newStep << " , ";
		}
	}
	outFile.close();
}

void FourierString::computeNewParameters(int currentStep)
{
	double time = getMeasureTimeFromStep(currentStep);
	currentFret = 0;
	oldTensionMod = newTensionMod;
	newTensionMod = 0;
	oldPickForce = newPickForce;
	newPickForce = 0;
	pulloffForce = 0;
	addedDamping.assign(overtones, 0);
	pickDisruption.assign(overtones, 0);
	pickResponse.assign(overtones, 0);
	//Finds any active fretting instructions and calculates the currentFret
	for (int i = 0; i < fretting.size(); i++) {
		if (fretting[i].start <= time && fretting[i].end >= time) {
			if (fretting[i].fretPosition > currentFret) {
				currentFret = fretting[i].fretPosition;
			}
		}
	}
	//fretScale accounts for the change in the effective "length" of the string when a fret is held
	double fretScale = pow(2, currentFret / (double) 12);
	//Any fretting instructions that ended recently are checked to see if they include an instruction for a pull-off
	for (int i = 0; i < fretting.size(); i++) {
		if (fretting[i].end <= time && getStepFromMeasureTime(fretting[i].end) + 300 >= currentStep && fretting[i].pull != 0) {
			pulloffForce = 0.002 * fretting[i].pull;
		}
	}
	//Checks for any active muting instructions
	for (int i = 0; i < muting.size(); i++) {
		//Muting starts slightly "too early" and fades in linearly, this avoids too much sudden change in tone
		if (muting[i].start - 0.5 <= time && muting[i].end - 0.1 >= time) {
			double press = 3;
			if (time < muting[i].start + 0.5) {
				press = 3 * (0.5 + time - muting[i].start);
			}
			for (int j = 0; j < overtones; j++) {
				//The amount of muting varies depending whether the muting point is a peak or node of the overtone in question, this is important for artificial harmonics
				addedDamping[j] += press * muting[i].damp * (muting[i].width * (1 + linearMuting * j) + abs(sin(3.1415 * (j + 1) * muting[i].location * fretScale)));
			}
		}
	}
	//Checks for any active bending instructions
	for (int i = 0; i < bending.size(); i++) {
		if (bending[i].start <= time && bending[i].start + bending[i].duration >= time) {
			if (bending[i].vibSpeed != 0 && bending[i].vibPolys.size() != 0) {
				//This handles vibrato, calls spline to interpolate the magnitude of vibrato from the discrete values of vibPolys
				newTensionMod += tension * 0.016 * spline(bending[i].vibPolys, (time - bending[i].start) / (double)bending[i].duration) * (1 + sin(0.3183 * currentStep / (double)bending[i].vibSpeed));
			}
			if (bending[i].bendPolys.size() != 0) {
				//This handles bends, calls spline to interpolate the depth of the bend from the discrete values of bendPolys
				double bendDistance = spline(bending[i].bendPolys, (time - bending[i].start) / (double)bending[i].duration);
				newTensionMod += tension * (pow(1 + bendDistance * bendDistance, 0.5) - 1);
			}
		}
	}
	//Checks for active picking instructions
	for (int i = 0; i < picking.size(); i++) {
		if (getStepFromMeasureTime(picking[i].start) - 200 <= currentStep && getStepFromMeasureTime(picking[i].start) + picking[i].delay - 200 <= currentStep
			&& getStepFromMeasureTime(picking[i].start) + 100 + picking[i].delay >= currentStep) {
			//Picking starts slightly "too early" so that the string is released and the tone produced closer to the actual beat
			newPickForce = (currentStep - getStepFromMeasureTime(picking[i].start) - picking[i].delay + 50) * 0.1 * picking[i].force / 150;
		}
	}
	//Natural damping is the inherent damping of the string, if the parameter highFretDamping is non-zero the natural damping increases when the string is fretted high
	for (int i = 0; i < overtones; i++) {
		naturalDamping[i] = constantDamping + (1 + highFretDamping * currentFret) * (linearDamping * i + quadraticDamping * i * i);
	}
}

std::vector<double> FourierString::matrixMultiply(const std::vector<std::vector<double>>& matrix, const std::vector<double>& input)
{
	//Multiplies matrices to apply the transition matrices to the state when changing frets
	int size = input.size();
	std::vector<double> output;
	output.assign(size, 0);
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			output[i] += input[j] * matrix[i][j];
		}
	}
	return output;
}

double FourierString::spline(const std::vector<double>& points, double input)
{
	//Interpolates between the values of the vector points by Gaussian smoothing
	if (points.size() > 1) {
		double scale = (double)points.size() - 1;
		double output = 0;
		double total = 0;
		double normalize = 0;
		for (int i = 0; i < points.size(); i++) {
			//250 is a magic number tuned to produce appropriate transition speeds for bends and vibrato
			normalize = 1.0 / (1 + 250 * (scale * input - i) * (scale * input - i));
			output += points[i] * normalize;
			total += normalize;
		}
		return output / total;
	}
	else if (points.size() == 1) {
		return points[0];
	}
	else { return 0; }
}

double FourierString::pickupResponse(double fretScale, int overtone) {
	//This function captures the effect of the pickup if the acoustic parameter is non-zero
	//The location of the pickup relative to the peaks and nodes of each frequency varies with fret, so fretScale must be passed to this function
	return 4 * (1 - acoustic) * (pickupWidth / (1 + overtone) + abs(sin(fretScale * 3.1415 * pickupLocation * (overtone + 1)))) * ( 1 + stringBrightness * overtone / (double) 100 ) + 3 * acoustic;
}