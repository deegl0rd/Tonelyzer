#include "Structures.h"
#include "Reader.h"
#include "Transformer.h"
#include "PitchAnalyzer.h"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Tonelyzer syntax: <executable_name> <input_file> [-dft] [-f=440] [-w=16384]" << std::endl;
		return 1;
	}

	// Inicializ�ci�s adatok
	const InitData init = GetInitData(argc, argv);

	// Olvas�
	AudioData read;
	try {
		read = Reader::ReadAudio(argv[1]);
	} 
	catch (const std::exception& e) 
	{
		std::cerr << e.what() << std::endl;
		std::cerr << "The file is either invalid or doesn't exist!" << std::endl;
		return 1;
	}

	// Fourier-transzform�ci�t v�gz� egys�g
	const Transformer tr(read, init.FTWindowSize);
	const FTdata output = tr.AvgFourier(init.FourierMode);

	// Hangmagass�g elemz� egys�g
	const PitchAnalyzer analyzer(read, output);
	const PitchHistogram histogram = analyzer.CalculateHistogram(init.ReferencePitch);
	const KeyPair key = analyzer.CalculateKeyKrumhansl(histogram);

	// Hangnem ki�rat�sa
	analyzer.PrintKeyKrumhansl(key);

	return 0;
}