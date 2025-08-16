#pragma once

#include <sndfile.h>

#include "Structures.h"

class Reader
{
public:
	static AudioData ReadAudio(const std::string path);
};

