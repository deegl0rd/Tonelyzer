#pragma once

#include "Transformer.h"

class PitchAnalyzer
{
public:
	PitchAnalyzer(const AudioData& audioData, const FTdata& fftResult);

	PitchHistogram CalculateHistogram(const float referenceFreq = 440.0f) const;
	KeyPair CalculateKeyKrumhansl(const PitchHistogram& histogram) const;
	void PrintKeyKrumhansl(const KeyPair& keyPair) const;

private:
	float GetProfileCorrelation(const PitchHistogram& histogram, const PitchHistogram& profile) const;
	static const PitchHistogram ShiftProfile(const PitchHistogram& profile, const int shiftAmount);
	static const std::string& GetPitchFromNumber(const unsigned pitch);

	const FTdata& fftResult;
	const AudioData& data;
	static const PitchNames pitchNames;
	static const PitchHistogram CmajorProfile;
	static const PitchHistogram CminorProfile;
};

