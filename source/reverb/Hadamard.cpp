#include "Hadamard.h"
#include <cmath>
#include <stdexcept>

/**
 * @brief Performs an in-place Hadamard transform on a JUCE audio buffer.
 *
 * The Hadamard transform is an orthogonal, fast transform using only
 * additions and subtractions. It is commonly used in feedback delay
 * networks and spatial audio processing.
 *
 * Each sample frame (across channels) is transformed independently.
 * The number of channels must be a power of 2.
 *
 * @param buffer The audio buffer to transform. Transformation is applied in-place.
 */
void Hadamard::process(juce::AudioBuffer<float>& buffer)
{
    int numChannels = buffer.getNumChannels();
    int numSamples = buffer.getNumSamples();

    if ((numChannels & (numChannels - 1)) != 0)
        throw std::invalid_argument("Number of channels must be a power of 2.");

    std::vector<float> temp(numChannels);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Collect samples from all channels at this sample index
        for (int channel = 0; channel < numChannels; ++channel)
            temp[channel] = buffer.getReadPointer(channel)[sample];

        // Fast Hadamard transform (in-place)
        for (size_t len = 1; len < temp.size(); len <<= 1)
        {
            for (size_t i = 0; i < temp.size(); i += (len << 1))
            {
                for (size_t j = 0; j < len; ++j)
                {
                    float a = temp[i + j];
                    float b = temp[i + j + len];
                    temp[i + j] = a + b;
                    temp[i + j + len] = a - b;
                }
            }
        }

        // Normalize the result
        float scale = 1.0f / std::sqrt(static_cast<float>(numChannels));
        for (auto& val : temp)
            val *= scale;

        // Write back the transformed samples to the buffer
        for (int channel = 0; channel < numChannels; ++channel)
            buffer.getWritePointer(channel)[sample] = temp[channel];
    }
}

/**
 * @brief Performs an in-place Hadamard transform on a float vector.
 *
 * The vector size must be a power of 2. Transformation uses only
 * additions and subtractions and is normalized by 1/sqrt(N).
 *
 * @param frame The float vector to transform in-place.
 */
void Hadamard::process(std::vector<float>& frame)
{
    size_t N = frame.size();
    if ((N & (N - 1)) != 0) return; // Must be power of 2

    // Fast Hadamard transform (in-place)
    for (size_t len = 1; len < N; len <<= 1)
    {
        for (size_t i = 0; i < N; i += (len << 1))
        {
            for (size_t j = 0; j < len; ++j)
            {
                float a = frame[i + j];
                float b = frame[i + j + len];
                frame[i + j] = a + b;
                frame[i + j + len] = a - b;
            }
        }
    }

    // Normalize
    float scale = 1.0f / std::sqrt(static_cast<float>(N));
    for (auto& val : frame)
        val *= scale;
}
