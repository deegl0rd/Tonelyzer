#pragma once

#include "Structures.h"

class Transformer
{
public:
	Transformer(const AudioData& audioData, const unsigned windowSize = 4096);

	void DFT(const FTdata& window, FTdata& result) const;
	void FFT(const FTdata& window, FTdata& result) const;

	FTdata AvgFourier(FTmode mode) const;
	FTdata AvgDFT() const;
	FTdata AvgFFT() const;

	void SetWindowSize(const unsigned int windowSize);
	inline unsigned int GetWindowSize() const { return windowSize; }

private:
	const AudioData& data;
	unsigned windowSize;
};

