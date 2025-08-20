#pragma once

// Standard library
#include <vector>
#include <memory>
#include <random>

// JUCE
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

// Project headers
#include "DelayLine.h"
#include "Hadamard.h"

/**
 * @class FDN
 * @brief Implements a Feedback Delay Network (FDN) for reverb and diffusion.
 *
 * The FDN class manages multiple delay lines with feedback and optional damping
 * via low-pass IIR filters. A Hadamard transform mixes the delay lines for
 * energy redistribution, creating a dense reverberation tail.
 *
 * The class is non-copyable due to unique_ptr storage, but supports move semantics.
 */
class FDN
{
public:
    /**
     * @brief Constructs an FDN with a given number of delay lines.
     * @param N Number of delay lines (typically equal to number of channels).
     * @param m Base delay length used to initialize each delay line.
     * @param blockSize Maximum block size for internal buffers.
     *
     * Each delay line's length is jittered randomly around m for decorrelation.
     * Random gains are assigned to each feedback path.
     */
    FDN(const int& N, const int& m, int blockSize);

    /** @brief Default constructor (produces empty, uninitialized FDN). */
    FDN() = default;

    /** @brief Destructor, cleans up internal delay lines and filters. */
    ~FDN() = default;

    // Copy operations: deleted (unique_ptr prevents safe copy)
    FDN(const FDN&) = delete;
    FDN& operator=(const FDN&) = delete;

    // Move operations: default (safe with unique_ptr)
    FDN(FDN&&) noexcept = default;
    FDN& operator=(FDN&&) noexcept = default;

    /**
     * @brief Processes an audio buffer through the FDN.
     * @param buffer Audio buffer to process in-place.
     * @param dampening Low-pass cutoff frequency for damping filters (Hz).
     * @param fs Sample rate of the audio buffer (Hz).
     * @param roomSize Scaling factor for perceived room size affecting delay indices.
     *
     * Each sample is read from the delay lines, mixed via a Hadamard transform,
     * filtered by the damping filters, and written back to the delay lines.
     * The final output replaces the input buffer contents.
     */
    void process(juce::AudioBuffer<float>& buffer,
        float dampening,
        double fs,
        float roomSize);

private:
    int N = 0; ///< Number of delay lines
    std::vector<int> M; ///< Delay line lengths
    std::vector<float> g; ///< Feedback gains per delay line

    std::vector<std::unique_ptr<DelayLine>> z; ///< Delay line storage
    std::vector<juce::dsp::IIR::Filter<float>> H; ///< Damping filters per delay line
};
