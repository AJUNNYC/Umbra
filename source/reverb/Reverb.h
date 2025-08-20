#pragma once

// JUCE
#include <JuceHeader.h>

// Project headers
#include "Diffuser.h"
#include "FDN.h"

// Standard library
#include <algorithm>
#include <vector>

/**
 * @class Reverb
 * @brief Implements a multi-stage reverb with diffusers, FDNs, and filtering.
 *
 * The Reverb class combines initial delays, diffusers (DVN-based),
 * feedback delay networks (FDNs), and low/high-pass filtering.
 * It supports stereo width adjustment, dry/wet mixing, and room size control.
 *
 * The processing chain is roughly:
 * initial delay -> diffuser1 -> FDN1 -> diffuser2 -> FDN2 -> diffuser3 -> stereo width -> dry/wet mix
 */
class Reverb
{
public:
    /**
     * @brief Constructs the Reverb with a given sample rate and block size.
     * @param fs Sample rate in Hz.
     * @param blockSize Maximum block size for internal buffers.
     *
     * Initializes initial delay lines, diffusers, FDNs, and filters.
     */
    Reverb(float fs, int blockSize);

    /** @brief Default constructor (produces uninitialized Reverb). */
    Reverb() = default;

    /** @brief Destructor. Cleans up DSP members. */
    ~Reverb() = default;

    // Copy operations: deleted to prevent accidental deep copies of internal buffers
    Reverb(const Reverb&) = delete;
    Reverb& operator=(const Reverb&) = delete;

    // Move operations: default, safe with JUCE and unique_ptr members
    Reverb(Reverb&&) noexcept = default;
    Reverb& operator=(Reverb&&) noexcept = default;

    /**
     * @brief Processes an audio buffer in-place with the reverb chain.
     * @param buffer Audio buffer to process (numChannels x numSamples).
     * @param mix Dry/wet mix (0.0 = dry, 1.0 = fully wet).
     * @param stereoWidth Stereo width factor (1.0 = original width, >1 = widened).
     * @param lowPass Low-pass filter cutoff frequency (Hz).
     * @param highPass High-pass filter cutoff frequency (Hz).
     * @param dampening FDN damping coefficient (controls tail absorption).
     * @param roomSize Scales delay indices for perceived room size.
     * @param initialDelay Initial pre-delay time (seconds) applied before reverb.
     *
     * The function updates the buffer in-place, including filtering, initial delay,
     * diffusers, FDNs, stereo width adjustment, and dry/wet mixing.
     */
    void process(juce::AudioBuffer<float>& buffer,
        float mix,
        float stereoWidth,
        float lowPass,
        float highPass,
        float dampening,
        float roomSize,
        float initialDelay);

private:
    float fs = 0.0f;          ///< Sample rate in Hz.
    int blockSize = 0;         ///< Maximum processing block size.

    // --- DSP members ---
    std::vector<DelayLine> z;  ///< Initial delay lines (pre-delays for each channel)
    Diffuser d1, d2, d3;       ///< Diffuser stages (DVN-based diffusion)
    FDN fdn1, fdn2;            ///< Feedback delay networks for late reverb

    std::vector<juce::IIRFilter> lowPassFilters;   ///< Per-channel low-pass filters (simulate HF absorption)
    std::vector<juce::IIRFilter> highPassFilters;  ///< Per-channel high-pass filters (remove subsonic rumble)
    float previousLowPass = 0.0f;   ///< Last applied low-pass cutoff
    float previousHighPass = 0.0f;  ///< Last applied high-pass cutoff
};
