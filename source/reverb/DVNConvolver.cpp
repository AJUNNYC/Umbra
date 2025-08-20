#include "DVNConvolver.h"
#include <algorithm>
#include <cmath>
#include <random>

/**
 * @brief Constructs a DVNConvolver with randomized pulse parameters.
 *
 * Generates M pulses with randomized widths, positions, and signs.
 * Each pulse is associated with an RRSFilter corresponding to its width.
 * The shared DelayLine z is sized to accommodate the latest pulse position.
 *
 * @param N Not used directly (future extension).
 * @param M Number of pulses to generate.
 * @param p Number of pulses per second (controls temporal density).
 * @param maxBlockSize Maximum audio block size.
 * @param fs Sample rate in Hz (used for timing grid calculation).
 */
DVNConvolver::DVNConvolver(int N, int M, int p, int maxBlockSize, double fs)
    : M(M), p(p)
{
    // Calculate segment length based on pulse density
    Td = static_cast<int>(fs / static_cast<double>(p));

    wmin = Td / 2 ;   // Minimum pulse width in samples
    wmax = Td;  // Maximum pulse width matches segment length

    // Resize pulse arrays
    k.resize(M);
    w.resize(M);
    s.resize(M);

    juce::Random rng;
    RRS.resize(wmax - wmin + 1); // One RRSFilter per possible pulse width

    // Generate randomized pulses
    for (int m = 0; m < M; ++m)
    {
        float r1 = rng.nextFloat();
        float r2 = rng.nextFloat();
        float r3 = rng.nextFloat();

        // Pulse width in [wmin, wmax]
        w[m] = static_cast<int>(std::round(r1 * (wmax - wmin) + wmin));

        // Pulse position within its segment
        k[m] = static_cast<int>(std::round(m * Td + r2 * (Td - w[m])));

        // Pulse sign ±1
        s[m] = 2 * static_cast<int>(std::round(r3)) - 1;

        // Assign pulse index to its RRSFilter group
        RRS[w[m] - wmin].second.push_back(m);
    }

    // Shared delay line sized to accommodate maximum pulse delay
    z = std::make_unique<DelayLine>(*std::max_element(k.begin(), k.end()), 0.0f, maxBlockSize);

    // Initialize one RRSFilter per width
    for (int w = 0; w < wmax - wmin + 1; ++w)
    {
        RRS[w].first = RRSFilter(wmin + w, 1.0f / 4096.0f, maxBlockSize);
    }

    // Temporary buffers for summing pulses
    sum1.resize(maxBlockSize, 0.0f);
    sum2.resize(maxBlockSize, 0.0f);
}

/**
 * @brief Processes a block of audio through the DVN convolution.
 *
 * The processing steps are:
 * 1. Write the input block into the shared delay line.
 * 2. Clear the accumulator buffer sum2.
 * 3. For each RRSFilter group (pulses of the same width):
 *    - Clear sum1 buffer.
 *    - Read each pulse from the delay line, multiply by its sign, and accumulate into sum1.
 *    - Process sum1 through the corresponding RRSFilter.
 *    - Accumulate the result into sum2.
 * 4. Copy sum2 back to the input block.
 * 5. Normalize the output to maintain consistent gain.
 *
 * @param block Pointer to the input audio block. Output is written in-place.
 * @param blockSize Number of samples in the block.
 */
void DVNConvolver::process(float* block, int blockSize)
{
    // Write input block into the shared delay line
    z->writeBlock(block, blockSize);

    // Clear final accumulator buffer
    juce::FloatVectorOperations::clear(sum2.data(), blockSize);

    // Process each RRSFilter group by pulse width
    for (int w = 0; w <= wmax - wmin; ++w)
    {
        // Clear temporary sum for this group
        juce::FloatVectorOperations::clear(sum1.data(), blockSize);

        // Sum pulses for this RRSFilter group
        for (int m : RRS[w].second)
        {
            // Read delayed pulse, multiply by its sign, and accumulate
            juce::FloatVectorOperations::addWithMultiply(
                sum1.data(),
                z->readBlock(k[m], blockSize),
                static_cast<float>(s[m]),
                blockSize
            );
        }

        // Apply the RRSFilter for this width
        RRS[w].first.process(sum1.data(), blockSize);

        // Accumulate filtered pulses into final output buffer
        juce::FloatVectorOperations::add(sum2.data(), sum1.data(), blockSize);
    }

    // Copy final accumulated signal to output block
    juce::FloatVectorOperations::copy(block, sum2.data(), blockSize);

    // Normalize output
    juce::FloatVectorOperations::multiply(
        block,
        1.0f / std::pow(float(M * (wmax - wmin + 1)), 19.0f / 30.0f),
        blockSize
    );
}
