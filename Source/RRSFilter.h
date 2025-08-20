#pragma once

// Standard library
#include <vector>

// Project headers
#include "DelayLine.h"

/**
 * @class RRSFilter
 * @brief Implements a Recursive Rectangular Smoothing (RRS) filter.
 *
 * This filter applies the RRS equation:
 * y[n] = (1 - epsilon)·y[n - 1] + x[n] - (1 - epsilon)^M·x[n - M]
 *
 * It uses internal delay lines for input and output to efficiently
 * implement the recursive feedback and feedforward.
 */
class RRSFilter
{
public:
    /**
     * @brief Constructs an RRSFilter with the given parameters.
     * @param M The filter order (delay in samples for x[n-M]).
     * @param epsilon_ The decay coefficient.
     * @param maxBlockSize Maximum block size for internal buffer allocations.
     */
    RRSFilter(int M, double epsilon_, int maxBlockSize);

    /**
     * @brief Default constructor.
     *
     * Creates an uninitialized filter. You must configure it before use.
     */
    RRSFilter() = default;

    /**
     * @brief Destructor.
     */
    ~RRSFilter() = default;

    // Copy operations are deleted because DelayLine contains internal buffers
    RRSFilter(const RRSFilter&) = delete;
    RRSFilter& operator=(const RRSFilter&) = delete;

    // Move operations are defaulted (safe for internal buffers)
    RRSFilter(RRSFilter&&) noexcept = default;
    RRSFilter& operator=(RRSFilter&&) noexcept = default;

    /**
     * @brief Processes a block of samples through the RRS filter.
     * @param block Pointer to an array of input samples. Output is written in-place.
     * @param blockSize Number of samples in the block.
     */
    void process(float* block, int blockSize);

private:
    DelayLine z_M;          /**< Delay line for x[n-M] */
    DelayLine z_1;          /**< Delay line for y[n-1] */

    double epsilon = 0.0;       /**< Filter decay coefficient */
    double epsilonM = 0.0;   /**< Precomputed (1-epsilon)^M for efficiency */
    int M = 0;                   /**< Filter order (number of delayed samples) */

    std::vector<float> y;        /**< Temporary output buffer for block processing */
};
