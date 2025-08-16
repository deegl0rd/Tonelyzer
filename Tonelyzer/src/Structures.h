#pragma once

#include <array>
#include <map>
#include <sstream>
#include <vector>
#include <complex>
#include <chrono>
#include <iostream>

//Konstans PI, a komplex matekhoz
const float PI = 3.1415927f;

enum FTmode
{
	FFT = 0,
	DFT = 1
};

struct AudioData
{
	bool SuccessfulRead = false;
	unsigned SampleRate = 0;
	unsigned Channels = 0;
	float referencePitch = 440.0f;
	std::vector<float> ReaderData;
	std::vector<float> MonoData;
	std::string Filename;
};

struct InitData
{
	FTmode   FourierMode = FTmode::FFT;
	float    ReferencePitch = 440.0f;
	unsigned FTWindowSize = 16384;
};

// A programban használt alias elnevezések
using FTdata = std::vector<std::complex<float>>;
using PitchHistogram = std::array<float, 12>;
using PitchNames = std::array<std::string, 12>;
using KeyPair = std::pair<int, int>;

inline InitData GetInitData(int argc, char* argv[])
{
	InitData data;
	for (int i = 0; i < argc; i++)
	{
		std::string cur = std::string(argv[i]);

		if (cur == "-dft") // DFT flag figyelõ
			data.FourierMode = FTmode::DFT;
		else if (cur.substr(0, 2) == "-f") // ReferencePitch-flag figyelõ
		{
			std::string freq_str;
			std::istringstream str(cur);
			std::getline(str, freq_str, '=');
			std::getline(str, freq_str, '=');
			data.ReferencePitch = static_cast<float>(std::atoi(freq_str.c_str()));
		}
		else if (cur.substr(0, 2) == "-w") // Window-flag figyelõ
		{
			std::string wins_str;
			std::istringstream str(cur);
			std::getline(str, wins_str, '=');
			std::getline(str, wins_str, '=');
			data.FTWindowSize = static_cast<unsigned>(std::atoi(wins_str.c_str()));
		}
	}

	return data;
}