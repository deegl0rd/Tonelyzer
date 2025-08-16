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

    if (data.Channels == 1) // Monó jel - 1/1 átmásolás
        data.MonoData = std::move(data.ReaderData);
    else if (data.Channels == 2) // Sztereó jel
    {
        data.MonoData.resize(static_cast<size_t>(sfInfo.frames));
        //Interleaved LR (Átlapolt bal-jobb csatorna), egyszerû átlagolás szerint monó jel képzése.
        for (size_t i = 0; i < sfInfo.frames; i++)
            data.MonoData[i] = (data.ReaderData[i * 2] + data.ReaderData[i * 2 + 1]) / 2.0f;
    }

    return data;
}
