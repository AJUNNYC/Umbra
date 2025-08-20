#pragma once

// Standard library
#include <vector>
#include <memory>
#include <utility>

// JUCE
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

// Project headers
#include "RRSFilter.h"
#include "DelayLine.h"

/**
 * @class DVNConvolver
 * @brief Implements a Dark Velvet Noise (DVN) convolution processor.
 *
 * The DVNConvolver generates a sparse pulse sequence with randomized
 * positions, widths, and signs. Each pulse is processed through a
 * DelayLine and an RRS (Recursive Random Sequence) filter corresponding
 * to its width. Pulses are summed and normalized to produce the output block.
 *
 * This class is non-copyable due to unique_ptr management but supports move semantics.
 */
class DVNConvolver
{
public:
    /**
     * @brief Constructs a DVNConvolver with randomized pulse parameters.
     * @param N Not used directly in the current implementation (future extension).
     * @param M Number of pulses to generate.
     * @param p Number of pulses per second (controls temporal density).
     * @param maxBlockSize Maximum expected audio block size.
     * @param fs Sample rate (used to calculate pulse timing grid).
     */
    DVNConvolver(int N, int M, int p, int maxBlockSize, double fs);

    /** @brief Default constructor (produces an empty, uninitialized convolver). */
    DVNConvolver() = default;

    /** @brief Destructor. Cleans up allocated resources. */
    ~DVNConvolver() = default;

    // Copy operations: deleted (unique_ptr prevents safe copy)
    DVNConvolver(const DVNConvolver&) = delete;
    DVNConvolver& operator=(const DVNConvolver&) = delete;

    // Move operations: default (safe with unique_ptr)
    DVNConvolver(DVNConvolver&&) noexcept = default;
    DVNConvolver& operator=(DVNConvolver&&) noexcept = default;

    /**
     * @brief Processes a block of audio samples using the DVN convolution.
     * @param block Pointer to the audio block to process in-place.
     * @param blockSize Number of samples in the block.
     *
     * Each pulse is read from its DelayLine position, multiplied by its sign,
     * processed through its corresponding RRS filter, summed, and normalized.
     */
    void process(float* block, int blockSize);

private:
    // --- Parameters ---
    int M = 0;      ///< Number of pulses
    int p = 0;      ///< Pulse density (pulses per second)
    int Td = 0;     ///< Time grid segment length (samples per segment)
    int wmin = 0;   ///< Minimum pulse width (samples)
    int wmax = 0;   ///< Maximum pulse width (samples)

    // --- Pulse data ---
    std::vector<int> k; ///< Pulse positions (sample offsets)
    std::vector<int> w; ///< Pulse widths (samples)
    std::vector<int> s; ///< Pulse signs (+1 or -1)

    // --- Processing components ---
    std::unique_ptr<DelayLine> z; ///< Shared DelayLine for all pulses
    std::vector<std::pair<RRSFilter, std::vector<int>>> RRS; ///< RRS filters grouped by width

    // --- Temporary buffers ---
    std::vector<float> sum1; ///< Temporary sum for each RRS filter group
    std::vector<float> sum2; ///< Final sum of all pulses before normalization
};
