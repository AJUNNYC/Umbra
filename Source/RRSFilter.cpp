#include "RRSFilter.h"
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <cstring>

using namespace juce;

/**
 * @brief Constructs an RRSFilter.
 *
 * Initializes the internal delay lines and precomputes (1 - epsilon)^M
 * for efficiency during block processing. Also allocates temporary buffer
 * for block processing.
 *
 * @param M Filter order (delay in samples for x[n-M]).
 * @param epsilon_ Decay coefficient (0 < epsilon < 1).
 * @param maxBlockSize Maximum block size for internal buffers.
 */
RRSFilter::RRSFilter(int M, double epsilon_, int maxBlockSize)
    : z_M(M, 0.0f, maxBlockSize),   // Delay line for x[n-M]
    z_1(1, 0.0f, maxBlockSize),   // Delay line for y[n-1]
    epsilon(epsilon_),
    epsilonM(std::pow(1.0f - epsilon_, static_cast<float>(M))),
    M(M)
{

    // Allocate temporary buffer for storing processed block
    y.resize(maxBlockSize, 0.0f);
}

/**
 * @brief Processes a block of samples through the RRS filter.
 *
 * Implements the recursive equation:
 * y[n] = (1 - epsilon) * y[n-1] + x[n] - (1 - epsilon)^M * x[n-M]
 *
 * Uses internal delay lines for efficiency:
 * - z_M stores the delayed input samples x[n-M].
 * - z_1 stores the previous output y[n-1].
 *
 * @param block Pointer to input samples. Output is written in-place.
 * @param blockSize Number of samples in the block.
 */
void RRSFilter::process(float* block, int blockSize)
{
    for (int i = 0; i < blockSize; ++i)
    {
        // Retrieve current input sample x[n]
        float x = block[i];         

        // Retrieve delayed input x[n-M] from z_M
        float x_M = z_M.readSample();

        // Retrieve previous output y[n-1] from z_1
        float y_1 = z_1.readSample();

        // Compute current output y[n] according to RRS formula
        float y = (x - epsilonM * x_M) + (1.0f - epsilon) * y_1;

        // Write current input x[n] into z_M for future M-delay
        z_M.writeSample(x);

        // Write current output y[n] into z_1 for next sample recursion
        z_1.writeSample(y);

        // Store result back into the block (in-place processing)
        block[i] = y;
    }
}
