#include "FourierString.h"
#include <iostream>

FourierString::FourierString(int tones, int number, std::string fileName)
{
	oldFret = 0;
	currentFret = 0;
	overtones = tones;
	stringNumber = number;
	int fretCount = 21;
	highFretDamping = 0.07;
	quadraticDamping = 0.012;
	linearDamping = 0.04;
	constantDamping = 3.7;
	overallNonlinearity = 0.005;
	highFrequencyNonlinearity = 0.007;
	pickingWidth = 14;
	pickHardness = 1.2;
	pickScratch = 0.02;
	pickingLocation = 0.171;
	linearMuting = 0.1;
	pickupWidth = 0.3;
	pickupLocation = 0.183;
	stringBrightness = 0.3;
	resonanceNum = 0;
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
	for (int i = 0; i < resonanceNum; i++) {
		std::vector<double> newVec;
		newVec.assign(overtones, 0);
		for (int j = 0; j < overtones; j++) {
			newVec[j] = cos(3.1415 * 0.1 * j * i);
		}
		resonateCoupling.push_back(newVec);
	}
	for (int i = 0; i < overtones; i++) {
		tensionModifiers[i] = (i + 1) * (i + 1) * ( 1 - 0.2 * i * i / (double) (overtones * overtones));
	}
	for (int i = 0; i < resonanceNum; i++) {
		resonanceDamping[i] = 32;
		resonanceFrequencies[i] = 30000 + 1600 * i + 700 * sqrt(i);
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
				for (int j = 0; j < overtones; j++) {
					nextMatrix[i][j] = 1 / (double) ( 1 + (i - j) * (i - j) );
				}
			}
			//This section rescales the transition matrices to guarantee that the pickup response is the same before and after the fret change
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
			if (dataType == "guitarPreset") {
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
							preset = dataType.append(1, nextChar);
						}
					}
				}
				std::ifstream presetStream(preset + ".txt");
				double presetFreq = 0;
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
			if (dataType == "updateFrequency") {
				inStream >> updatesPerSubdivision;
			}
			if (dataType == "highFretDamping") {
				inStream >> highFretDamping;
			}
			if (dataType == "quadraticDamping") {
				inStream >> quadraticDamping;
			}
			if (dataType == "linearDamping") {
				inStream >> linearDamping;
			}
			if (dataType == "constantDamping") {
				inStream >> constantDamping;
			}
			if (dataType == "overallNonlinearity") {
				inStream >> overallNonlinearity;
			}
			if (dataType == "highFrequencyNonlinearity") {
				inStream >> highFrequencyNonlinearity;
			}
			if (dataType == "pickingWidth") {
				inStream >> pickingWidth;
			}
			if (dataType == "pickHardness") {
				inStream >> pickHardness;
			}
			if (dataType == "pickScratch") {
				inStream >> pickScratch;
			}
			if (dataType == "pickingLocation") {
				inStream >> pickingLocation;
			}
			if (dataType == "linearMuting") {
				inStream >> linearMuting;
			}
			if (dataType == "pickupWidth") {
				inStream >> pickupWidth;
			}
			if (dataType == "pickupLocation") {
				inStream >> pickupLocation;
			}
			if (dataType == "stringBrightness") {
				inStream >> stringBrightness;
			}
			if (dataType == "resonanceNumber") {
				inStream >> resonanceNum;
			}
			if (dataType == "String0Freq") {
				double freq;
				inStream >> freq;
				if (stringNumber == 0) {
					tension = 40 * freq * freq;
				}
			}
			if (dataType == "String1Freq") {
				double freq;
				inStream >> freq;
				if (stringNumber == 1) {
					tension = 40 * freq * freq;
				}
			}
			if (dataType == "String2Freq") {
				double freq;
				inStream >> freq;
				if (stringNumber == 2) {
					tension = 40 * freq * freq;
				}
			}
			if (dataType == "String3Freq") {
				double freq;
				inStream >> freq;
				if (stringNumber == 3) {
					tension = 40 * freq * freq;
				}
			}
			if (dataType == "String4Freq") {
				double freq;
				inStream >> freq;
				if (stringNumber == 4) {
					tension = 40 * freq * freq;
				}
			}
			if (dataType == "String5Freq") {
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
				inBend.push_back(0);
				if (string == stringNumber) {
					bending.push_back(bend(start, duration, 600, inVib, inBend));
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
	if (currentFret != oldFret) {
		State = matrixMultiply(fretChangeMatrix[oldFret][currentFret], State);
		oldFret = currentFret;
	}
	pickupValue = 0;
	double effectiveTension = (tension + currentTensionMod) * pow(2, (double)currentFret / 6);
	double fretScale = pow(2, currentFret / (double)12);
	for (int j = 0; j < stepsPerSample; j++) {
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
		for (int i = 0; i < resonanceNum; i++) {
			for (int k = 0; k < overtones; k++) {
				resonances[i] += resonateCoupling[i][k] * timeStep * Derivative[k] / (1 + k * k);
			}
			resonanceDerivatives[i] += -timeStep * (resonanceFrequencies[i] * resonances[i] + resonanceDamping[i] * resonanceDerivatives[i]);
			resonances[i] += timeStep * resonanceDerivatives[i];
			for (int k = 0; k < overtones; k++) {
				Derivative[k] += resonateCoupling[i][k] * timeStep * 0.16 * resonanceDerivatives[i] / (1 + k * k);
			}
		}
		for (int q = 0; q < 8; q++) {
			if (currentPickForce != 0) {
				double pickSpot = 0;
				for (int i = 0; i < overtones; i++) {
					pickSpot += sin((pickingLocation + 0.003 * q) * 3.1415 * (i + 1) * fretScale) * State[i] * (pickingWidth + pickHardness * i + pickScratch * i * i);
				}
				if (pickSpot < currentPickForce) {
					for (int i = 0; i < overtones; i++) {
						//Derivative[i] += (currentPickForce - pickSpot) / (1 + (currentPickForce - pickSpot)) * (200000 + tension + effectiveTension) * sin((pickingLocation) * fretScale * 3.1415 * (i + 1)) * (pickingWidth + pickHardness * i + pickScratch * i * i) / 10000000;
						Derivative[i] += sin( (pickingLocation + 0.003 * q) * fretScale * 3.1415 * (i + 1)) * (500000 + tension + effectiveTension) * (pickingWidth + pickHardness * i + pickScratch * i * i) / 800000000;
					}
				}
			}
		}
		if (pulloffForce != 0) {
			for (int i = 0; i < overtones; i++) {
				//Derivative[i] += (800000 + tension + effectiveTension) * 0.1 * (pulloffForce - State[i]) * (20 + i) / 1000000;
				Derivative[i] += 0.1 * (500000 + tension + effectiveTension) * (pulloffForce - State[i]) * (20 + i) / 800000000;
			}
		}
	}
	for (int i = 0; i < overtones; i++) {
		pickupValue += pickupResponse(fretScale, i) * State[i];
	}
	return pickupValue;
}

void FourierString::updateParameters(double weight)
{
	currentTensionMod = (1 - weight) * oldTensionMod + weight * newTensionMod;
	currentPickForce = (1 - weight) * oldPickForce + weight * newPickForce;
}

int FourierString::getStepFromMeasureTime(double time) {
	return (int)round(time * 44100 / (tempo * subdivision));
}

double FourierString::getMeasureTimeFromStep(int step)
{
	return (tempo * subdivision) * (double)step / 44100;
}

void FourierString::simulate(std::string file)
{
	double time = 0;
	double output = 0;
	double weight = 0;
	std::ofstream outFile(file);
	double newStep = 0;
	for (int currentStep = 0; currentStep < totalMusicLength + 22050; currentStep++) {
		newStep = 0;
		time = getMeasureTimeFromStep(currentStep);
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
	for (int i = 0; i < fretting.size(); i++) {
		if (fretting[i].start <= time && fretting[i].end >= time) {
			if (fretting[i].fretPosition > currentFret) {
				currentFret = fretting[i].fretPosition;
			}
		}
	}
	double fretScale = pow(2, currentFret / (double) 12);
	for (int i = 0; i < fretting.size(); i++) {
		if (fretting[i].end <= time && getStepFromMeasureTime(fretting[i].end) + 300 >= currentStep && fretting[i].pull != 0) {
			pulloffForce = 0.002 * fretting[i].pull;
		}
	}
	for (int i = 0; i < muting.size(); i++) {
		if (muting[i].start - 0.5 <= time && muting[i].end - 0.1 >= time) {
			double press = 3;
			if (time < muting[i].start + 0.5) {
				press = 3 * (0.5 + time - muting[i].start);
			}
			for (int j = 0; j < overtones; j++) {
				addedDamping[j] += press * muting[i].damp * (muting[i].width + abs(sin(3.1415 * (j + 1) * muting[i].location * fretScale)) + linearMuting * j);
			}
		}
	}
	for (int i = 0; i < bending.size(); i++) {
		if (bending[i].start <= time && bending[i].start + bending[i].duration >= time) {
			if (bending[i].vibPolys.size() != 0) {
				newTensionMod += tension * 0.016 * spline(bending[i].vibPolys, (time - bending[i].start) / (double)bending[i].duration) * sin(0.5 * currentStep / (double)bending[i].vibSpeed);
			}
			if (bending[i].bendPolys.size() != 0) {
				double bendDistance = spline(bending[i].bendPolys, (time - bending[i].start) / (double)bending[i].duration);
				newTensionMod += tension * (pow(1 + bendDistance * bendDistance, 0.5) - 1);
			}
		}
	}
	for (int i = 0; i < picking.size(); i++) {
		if (getStepFromMeasureTime(picking[i].start) - 200 <= currentStep && getStepFromMeasureTime(picking[i].start) + picking[i].delay - 200 <= currentStep
			&& getStepFromMeasureTime(picking[i].start) + 100 + picking[i].delay >= currentStep) {
			newPickForce = (currentStep - getStepFromMeasureTime(picking[i].start) - picking[i].delay + 50) * 0.1 * picking[i].force / 150;
		}
	}
	for (int i = 0; i < overtones; i++) {
		naturalDamping[i] = constantDamping + (1 + highFretDamping * currentFret) * (linearDamping * i + quadraticDamping * i * i);
	}
}

std::vector<double> FourierString::matrixMultiply(const std::vector<std::vector<double>>& matrix, const std::vector<double>& input)
{
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
	if (points.size() > 1) {
		double scale = (double)points.size() - 1;
		double output = 0;
		double total = 0;
		double normalize = 0;
		for (int i = 0; i < points.size(); i++) {
			normalize = 1.0 / (1 + 100 * (scale * input - i) * (scale * input - i));
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
	return 4 * (pickupWidth + abs(sin(fretScale * 3.1415 * pickupLocation * (overtone + 1)))) * ( 1 + stringBrightness * overtone / (double) 100 );
	//return 2 * (1 + stringBrightness * overtone / (double)overtones);
}
