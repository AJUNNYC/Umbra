#include "DelayLine.h"
#include <stdexcept>
#include <cstring>

/**
 * @brief Constructs a DelayLine.
 *
 * Initializes the circular buffer with a mirrored layout to simplify block processing.
 * @param M Maximum delay in samples.
 * @param g Gain applied to delayed samples (for feedback processing).
 * @param maxBlockSize Maximum expected processing block size.
 * @throws std::invalid_argument if M < 0 or maxBlockSize <= 0
 */
DelayLine::DelayLine(int M, float g, int maxBlockSize)
    : M(M), g(g)
{
    if (M < 0 || maxBlockSize <= 0)
        throw std::invalid_argument("Delay length and block size must be positive");

    // Buffer size: max delay + max block size (ensures block reads never overflow)
    bufferSize = M + maxBlockSize;

    // Mirror buffer: store two consecutive copies of the delay line
    // This allows readBlock to always return a contiguous memory block
    buffer.resize(2 * bufferSize, 0.0f);

    // Start writing at index 0
    write = 0;
}

// --- Block-based API ---

/**
 * @brief Reads a block of delayed samples from the buffer.
 *
 * Computes the read index with wrap-around for the circular buffer.
 * Uses the mirrored buffer to guarantee contiguous memory for block operations.
 * @param tau Delay in samples (0..M)
 * @param blockSize Number of samples to read
 * @return Pointer to the block of samples (mutable for DSP operations)
 * @throws std::out_of_range if tau or blockSize is invalid
 */
float* DelayLine::readBlock(const int tau, const int blockSize) const {
    if (tau < 0 || tau > M)
        throw std::out_of_range("Tau exceeds maximum delay");

    if (blockSize <= 0 || blockSize > bufferSize - tau)
        throw std::out_of_range("Block size out of bounds");

    // FIXED: Calculate where to read from based on delay and block size
    // Read from where the write started (write - blockSize), minus the delay
    int read = write - tau - blockSize;
    if (read < 0)
        read += bufferSize; // wrap-around for circular buffer

    // Return pointer into mirrored buffer to allow contiguous block read
    return const_cast<float*>(&buffer[read]);
}

/**
 * @brief Reads a block using the maximum delay M.
 */
float* DelayLine::readBlock(const int blockSize) const {
    return readBlock(M, blockSize);
}

/**
 * @brief Writes a block of samples into the delay line.
 *
 * Handles wrap-around at the end of the circular buffer.
 * Maintains mirrored buffer for contiguous block reads.
 * @param input Pointer to input samples
 * @param blockSize Number of samples to write
 */
void DelayLine::writeBlock(const float* input, const int blockSize) {
    if (!input || blockSize <= 0)
        return;

    if (write + blockSize <= bufferSize) {
        // Simple contiguous copy (no wrap)
        std::memcpy(&buffer[write], input, blockSize * sizeof(float));
        std::memcpy(&buffer[write + bufferSize], input, blockSize * sizeof(float));
    }
    else {
        // Wrap-around copy
        int firstPart = bufferSize - write;
        int secondPart = blockSize - firstPart;

        std::memcpy(&buffer[write], input, firstPart * sizeof(float));
        std::memcpy(&buffer[0], input + firstPart, secondPart * sizeof(float));

        // Mirror buffer
        std::memcpy(&buffer[write + bufferSize], input, firstPart * sizeof(float));
        std::memcpy(&buffer[bufferSize], input + firstPart, secondPart * sizeof(float));
    }

    // Advance write index circularly
    write = (write + blockSize) % bufferSize;
}

/**
 * @brief Processes a block in-place (writes and reads delayed output into the same block).
 */
void DelayLine::processBlock(const float* input, const int blockSize) {
    if (!input || blockSize <= 0)
        return;

    // Write input into delay line
    writeBlock(input, blockSize);

    // Overwrite input with delayed output (maximum delay)
    std::memcpy(const_cast<float*>(input), readBlock(blockSize), blockSize * sizeof(float));
}

// --- Sample-based API ---

/**
 * @brief Reads a delayed sample at delay tau.
 *
 * Wraps around the circular buffer automatically.
 * @param tau Delay in samples
 * @return Sample value at time n - tau
 */
float DelayLine::readSample(const int tau) const {
    if (tau > M || tau < 0)
        throw std::out_of_range("Tau exceeds maximum delay");

    int read = write - tau - 1;
    if (read < 0)
        read += bufferSize; // wrap-around

    return buffer[read];
}

/**
 * @brief Reads the sample at maximum delay M.
 */
float DelayLine::readSample() const {
    return readSample(M);
}

/**
 * @brief Writes a single sample into the delay line.
 *
 * Also updates the mirrored buffer for contiguous block operations.
 * @param input Sample to write
 */
void DelayLine::writeSample(const float input) {
    buffer[write] = input;
    buffer[write + bufferSize] = input; // mirror write
    write = (write + 1) % bufferSize;
}

/**
 * @brief Processes a single sample through the delay line with feedback.
 *
 * Implements a one-pole comb filter: y[n] = x[n] + g * y[n - M]
 * Stores the result into the buffer and mirrored buffer.
 * @param input Sample to process
 */
void DelayLine::processSample(const float& input) {
    float x = input;
    float y = readSample(M);      // delayed sample at maximum delay
    float z = x + g * y;          // apply feedback gain

    buffer[write] = z;            // write to main buffer
    buffer[write + bufferSize] = z; // write to mirrored buffer
    write = (write + 1) % bufferSize; // advance circular index
}
