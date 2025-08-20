#pragma once

// Standard library
#include <vector>
#include <memory>
#include <stdexcept>

// JUCE
#include <JuceHeader.h>

// Project headers
#include "DVNConvolver.h"

/**
 * @class Diffuser
 * @brief Applies multiple DVNConvolver instances to simulate multi-channel diffusion.
 *
 * The Diffuser manages a collection of DVNConvolver objects, one per audio channel.
 * Each convolver independently applies a dark velvet noise convolution to its channel.
 * This can be used to increase spatial richness or create diffusion effects in reverberation.
 *
 * The class is non-copyable due to unique_ptr storage, but supports move semantics.
 */
class Diffuser
{
public:
    /**
     * @brief Constructs a Diffuser with N channels, each with a DVNConvolver.
     * @param N Number of audio channels.
     * @param M Number of pulses per DVNConvolver.
     * @param p Pulses per second per DVNConvolver.
     * @param blockSize Maximum audio block size.
     * @param fs Sample rate (Hz) used for pulse timing calculation.
     * @throws std::invalid_argument if N < 1.
     */
    Diffuser(const int& N, int M, int p, int blockSize, double fs);

    /** @brief Default constructor (produces empty uninitialized Diffuser). */
    Diffuser() = default;

    /** @brief Destructor. Cleans up internal DVNConvolver instances. */
    ~Diffuser() = default;

    // Copy operations: deleted (unique_ptr prevents safe copy)
    Diffuser(const Diffuser&) = delete;
    Diffuser& operator=(const Diffuser&) = delete;

    // Move operations: default (safe with unique_ptr)
    Diffuser(Diffuser&&) noexcept = default;
    Diffuser& operator=(Diffuser&&) noexcept = default;

    /**
     * @brief Processes an audio buffer with all internal DVNConvolver instances.
     * @param buffer Audio buffer to process in-place. Must have at least N channels.
     *
     * Each channel is processed independently using its corresponding DVNConvolver.
     * This produces a diffused output that preserves the per-channel structure.
     */
    void process(juce::AudioBuffer<float>& buffer);

private:
    int N = 0; ///< Number of channels
    std::vector<std::unique_ptr<DVNConvolver>> dvnConvolvers; ///< One DVNConvolver per channel
};
