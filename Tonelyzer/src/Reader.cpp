#include "Reader.h"

AudioData Reader::ReadAudio(const std::string path)
{
    AudioData data;

    SNDFILE* file;
    SF_INFO sfInfo;
    file = sf_open(path.c_str(), SFM_READ, &sfInfo);

    if (!file)
        throw std::exception();

    data.SuccessfulRead = true;
    data.Filename = path;
    data.SampleRate = sfInfo.samplerate;
    data.Channels = sfInfo.channels;

    data.ReaderData.resize(static_cast<size_t>(sfInfo.frames * sfInfo.channels));
    sf_read_float(file, data.ReaderData.data(), data.ReaderData.size());
    sf_close(file);

    if (data.Channels == 1) // Mon� jel - 1/1 �tm�sol�s
        data.MonoData = std::move(data.ReaderData);
    else if (data.Channels == 2) // Sztere� jel
    {
        data.MonoData.resize(static_cast<size_t>(sfInfo.frames));
        //Interleaved LR (�tlapolt bal-jobb csatorna), egyszer� �tlagol�s szerint mon� jel k�pz�se.
        for (size_t i = 0; i < sfInfo.frames; i++)
            data.MonoData[i] = (data.ReaderData[i * 2] + data.ReaderData[i * 2 + 1]) / 2.0f;
    }

    return data;
}
