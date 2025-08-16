#include "Transformer.h"

Transformer::Transformer(const AudioData& audioData, const unsigned windowSize)
	: data(audioData)
{
	try 
	{
		SetWindowSize(windowSize);
	}
	catch (const std::out_of_range& e) 
	{
		std::cerr << e.what() << std::endl;
		std::cerr << "DFT / FFT window size is out of bounds!" << std::endl;
		this->windowSize = InitData().FTWindowSize;
		SetWindowSize(windowSize);
	}
	catch (const std::invalid_argument& e) 
	{
		std::cerr << e.what() << std::endl;
		std::cerr << "DFT / FFT window size is not a power of two!" << std::endl;
		this->windowSize = InitData().FTWindowSize;
		SetWindowSize(windowSize);
	}
}

// A Fourier-transzform�ci�t elv�gz� algoritmusok a https://mogi.bme.hu/TAMOP/mereselmelet/ch11.html#ch-XI.5
// weblap szerint lettek implement�lva

// Val�s Diszkr�t Fourier-transzform�ci� meghat�rozott m�ret� ablakra. 
// Sz�m�t�si bonyolults�ga: O(n^2) Nagy ablakm�retn�l nagyon lass�!
void Transformer::DFT(const FTdata& window, FTdata& result) const
{
	result.resize(window.size());
	for (size_t k = 0; k < window.size(); k++)
	{
		std::complex<float> S = 0.0f;

		for (size_t n = 0; n < window.size(); n++)
			S += window[n] * std::polar<float>(1.0f, (-2.0f * PI * k * n / window.size()));

		result[k] = S;
	}
}

// Rekurz�v Cooley-Tukey f�le Gyors Fourier-transzform�ci� (FFT) meghat�rozott m�ret� ablakra.
// Sz�m�t�si bonyolults�ga: O(n*log2(n)). Sokkal gyorsabb, �s nagyobb ablakm�reteket is elb�r!
void Transformer::FFT(const FTdata& window, FTdata& result) const
{
	if (window.size() <= 1)
	{
		result = window;
		return;
	};

	const size_t halfSize = window.size() / 2;

	FTdata even(halfSize), odd(halfSize);
	for (size_t i = 0; i < halfSize; i++) 
	{
		even[i] = window[i * 2];
		odd[i]  = window[i * 2 + 1];
	}

	// Rekurz�v FFT-ablakok futtat�sa
	FTdata trEven(halfSize), trOdd(halfSize);
	FFT(even, trEven);
	FFT(odd, trOdd);

	result.resize(window.size());
	for (size_t k = 0; k < halfSize; k++) 
	{
		std::complex<float> t = std::polar(1.0f, -2.0f * PI * k / window.size()) * trOdd[k];
		result[k]			  = trEven[k] + t;
		result[k + halfSize]  = trEven[k] - t;
	}
}

// Generikus Fourier-transzform�ci� rutin.
FTdata Transformer::AvgFourier(FTmode mode) const
{
	std::cout << "Tonelyzer: Processing " << data.Filename << " in " << (mode == FTmode::DFT ? "DFT" : "FFT") << " mode. " << std::endl;	
	std::cout << "--------------------------------" << std::endl;

	const float overlapFactor = 0.5f;
	size_t runs = 0;
	float runtime = 0.0f;
	size_t totalRuns = static_cast<size_t>(std::floor(data.MonoData.size() / (windowSize * overlapFactor)));

	FTdata out(windowSize, 0.0f);

	for (size_t i = 0; i + windowSize < data.MonoData.size(); i += static_cast<size_t>(overlapFactor * windowSize), runs++)
	{
		auto before = std::chrono::high_resolution_clock::now(); // Id�m�r�s

		// Jelenlegi ablak kiv�laszt�sa
		FTdata window(data.MonoData.begin() + i, data.MonoData.begin() + i + windowSize);
		FTdata result(windowSize, 0.0f);

		for (size_t j = 0; j < window.size(); j++)
			window[j] *= 0.5f * (1.0f - std::cos(2.0f * PI * j / (windowSize - 1)));

		if (mode == FTmode::FFT)
			FFT(window, result);
		else if (mode == FTmode::DFT)
			DFT(window, result);

		for (size_t j = 0; j < result.size(); j++)
			out[j] += result[j] * (1.0f / totalRuns);

		auto after = std::chrono::high_resolution_clock::now();
		auto dur = std::chrono::duration_cast<std::chrono::microseconds>(after - before).count() / 1000.0f;
		runtime += dur;

		// 50 fut�s ut�n v�rhat� id�tartam kijelz�se a felhaszn�l�nak
		if (runs == 50)
		{
			float estimatedSeconds = (totalRuns * (runtime / runs)) / 1000.0f;
			std::cout << "Estimated finish time: " << estimatedSeconds << "s\n";
		}
	}

	float avgTime = runtime / runs;
	std::cout << runs << " FFT windows in total, elapsed: " << runtime / 1000.0f << "s, time/window: " << avgTime << "ms\n";
	std::cout << "--------------------------------" << std::endl;

	return out;
}

// Lefuttatja a teljes f�jlra a DFT-t, majd �tlagolja a kapott spektrumot.
FTdata Transformer::AvgDFT() const
{	
	return AvgFourier(FTmode::DFT);
}

// Lefuttatja a teljes f�jlra az FFT-t, majd �tlagolja a kapott spektrumot.
FTdata Transformer::AvgFFT() const
{
	return AvgFourier(FTmode::FFT);
}

void Transformer::SetWindowSize(const unsigned int windowSize)
{
	//Ablakm�ret korl�toz�sok, 128 �s 32768 k�z�tt, �s 2^x kell legyen!

	if (windowSize < 128)
		throw std::out_of_range("DFT/FFT window size is too small! Minimum value is 128");
	if (windowSize > 32768)
		throw std::out_of_range("DFT/FFT window size is too big! Maximum value is 32768");
	
	bool isPowerOfTwo = false;
	for (int i = 1; i <= 15; i++) // 2^15 = 32768
	{
		if (windowSize == pow(2, i))
			isPowerOfTwo = true;
	}

	if (isPowerOfTwo)
		this->windowSize = windowSize;
	else
		throw std::invalid_argument("DFT/FFT window size is not a power of two!");
}
