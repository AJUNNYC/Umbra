#include "Diffuser.h"
#include <stdexcept>
#include <omp.h>
#include <iostream>

/**
 * @brief Constructs a Diffuser with a DVNConvolver per channel.
 *
 * Each DVNConvolver independently applies dark velvet noise convolution
 * to its respective channel, producing a diffused, spatially rich output.
 *
 * @param N Number of audio channels.
 * @param M Number of pulses per DVNConvolver.
 * @param p Pulse density (pulses per second).
 * @param blockSize Maximum expected audio block size.
 * @param fs Sample rate used for pulse timing.
 *
 * @throws std::invalid_argument if N < 1.
 */
Diffuser::Diffuser(const int& N, int M, int p, int blockSize, double fs) : N(N)
{
    if (N < 1)
        throw std::invalid_argument("Number of channels must be at least 1.");

    dvnConvolvers.resize(N);

    for (int channel = 0; channel < N; ++channel)
    {
        // Construct a DVNConvolver for each channel
        // Note: Each convolver is independent and manages its own pulse sequence
        dvnConvolvers[channel] = std::make_unique<DVNConvolver>(N, M, p, blockSize, fs);
    }
}

/**
 * @brief Processes an audio buffer through all DVNConvolver instances.
 *
 * Steps:
 * 1. Iterate over each channel.
 * 2. Obtain a writable pointer to the channel data.
 * 3. Process the block in-place using the corresponding DVNConvolver.
 * 4. Output is written back directly into the buffer.
 *
 * @param buffer The audio buffer to process. Must contain at least N channels.
 */
void Diffuser::process(juce::AudioBuffer<float>& buffer)
{
#pragma omp parallel for
    for (int channel = 0; channel < N; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        // Process the current channel with its DVNConvolver
        dvnConvolvers[channel]->process(channelData, buffer.getNumSamples());
    }

    // Note: If the input buffer has more channels than N, remaining channels are unchanged.
    // If fewer channels, consider upmixing or handling in calling code.
}

