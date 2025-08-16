#include "PitchAnalyzer.h"

const PitchNames PitchAnalyzer::pitchNames
{
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

const PitchHistogram PitchAnalyzer::CmajorProfile
{
    6.35f, 2.23f, 3.48f, 2.33f, 4.38f, 4.09f, 2.52f, 5.19f, 2.39f, 3.66f, 2.29f, 2.88f
};

const PitchHistogram PitchAnalyzer::CminorProfile
{
    6.33f, 2.68f, 3.52f, 5.38f, 2.60f, 3.53f, 2.54f, 4.75f, 3.98f, 2.69f, 3.34f, 3.17f
};

PitchAnalyzer::PitchAnalyzer(const AudioData& audioData, const FTdata& fftResult)
    : fftResult(fftResult), data(audioData) {}


PitchHistogram PitchAnalyzer::CalculateHistogram(const float referencePitch) const
{
    const size_t halfSize = fftResult.size() / 2;
    std::array<float, 12> histogram;
    histogram.fill(0);

    for (size_t k = 1; k < halfSize; k++)
    {
        float f = k * data.SampleRate / (float) fftResult.size();
        if (f < 20.0f || f > 5000.0f) continue;

        float midi = 69.0f + 12.0f * std::log2(f / referencePitch);

        unsigned loMidi = static_cast<unsigned>(std::floor(midi));
        unsigned hiMidi = loMidi + 1;
        float frac = std::fmod(midi, 1.0f);
        float ampl = std::abs(fftResult[k]);

        if (loMidi >= 0)
        {
            unsigned loPitch = static_cast<unsigned>(loMidi) % 12;
            histogram[loPitch] += ampl * (1.0f - frac);
        }

        unsigned hiPitch = static_cast<unsigned>(std::round(midi)) % 12;
        histogram[hiPitch] += ampl * frac;
    }

    return histogram;
}

KeyPair PitchAnalyzer::CalculateKeyKrumhansl(const PitchHistogram& histogram) const
{
    float bestMatch = -FLT_MAX;

    int key_PitchIndex = 0; // 0-C -> 11-B
    int key_ScaleIndex = 1; // 0: moll, 1: dúr

    // Dúr skálák ellenõrzése
    for (int i = 0; i < 12; i++)
    {
        const PitchHistogram curKey = ShiftProfile(CmajorProfile, i);
        float correlation = GetProfileCorrelation(histogram, curKey);

        if (bestMatch < correlation)
        {
            bestMatch = correlation;
            key_PitchIndex = i;
            key_ScaleIndex = 1;
        }
    }

    // Moll skálák ellenõrzése
    for (int i = 0; i < 12; i++)
    {
        const PitchHistogram curKey = ShiftProfile(CminorProfile, i);
        float correlation = GetProfileCorrelation(histogram, curKey);

        if (bestMatch < correlation)
        {
            bestMatch = correlation;
            key_PitchIndex = i;
            key_ScaleIndex = 0;
        }
    }

    return std::pair<int, int>(key_PitchIndex, key_ScaleIndex);
}

void PitchAnalyzer::PrintKeyKrumhansl(const KeyPair& keyPair) const
{
    std::string pitch;
    try 
    {
        pitch = GetPitchFromNumber(keyPair.first);
    }
    catch (const std::out_of_range& e) 
    {
        std::cerr << e.what() << std::endl;
        std::cerr << "Invalid pitch number! Valid pitch number is from 0 (C) to 11 (B). Displaying pitch 'C'." << std::endl;
        pitch = pitchNames[0];
    }

    std::cout << "Estimated key: " << pitch << " ";
    std::cout << (keyPair.second == 1 ? "major" : "minor") << std::endl;
}

float PitchAnalyzer::GetProfileCorrelation(const PitchHistogram& histogram, const PitchHistogram& profile) const
{
    float histogramSum = 0.0f;
    for (const float value : histogram)
        histogramSum += value;
    const float histogramMean = histogramSum / 12.0f;

    float profileSum = 0.0f;
    for (const float value : profile)
        profileSum += value;
    const float profileMean = profileSum / 12.0f;

    float num = 0.0f;
    float den = 0.0f;
    float den_deviationHistogram2 = 0.0f;
    float den_deviationProfile2   = 0.0f;

    for (int i = 0; i < 12; i++)
    {
        // Mennyire térnek el a hisztogram egyes elemei az átlagától.
        const float deviationHistogram = histogram[i] - histogramMean;    
        // Mennyire térnek el a skálaprofil adott hangjai az átlagától.
        const float deviationProfile = profile[i] - profileMean;

        num += deviationHistogram * deviationProfile;
        den_deviationHistogram2 += deviationHistogram * deviationHistogram;
        den_deviationProfile2   += deviationProfile * deviationProfile;
    }

    den = std::sqrt(den_deviationHistogram2 * den_deviationProfile2);

    return num / den;
}

const PitchHistogram PitchAnalyzer::ShiftProfile(const PitchHistogram& profile, const int shiftAmount)
{
    // Ha épp nem toljuk el a skálaprofilt, adjuk vissza az eredetit.
    if (shiftAmount == 0) return profile;

    // Eltolja a relatív skálaprofilokat az éppen vizsgált alaphangra.
    PitchHistogram shifted;
    for (int i = 0; i < 12; i++)
        shifted[i] = profile[(i + 12 - shiftAmount) % 12];
    return shifted;
}

const std::string& PitchAnalyzer::GetPitchFromNumber(const unsigned pitch)
{
    if (pitch > 11)
        throw std::out_of_range("Invalid pitch number! Valid pitch number is from 0 (C) to 11 (B)");

    return pitchNames.at(pitch);
}
