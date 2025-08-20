#pragma once

// Standard library
#include <vector>

// Forward declarations
class FDN;

/**
 * @class DelayLine
 * @brief Implements a simple circular delay line for block and sample processing.
 *
 * The DelayLine supports both block-based and sample-based APIs. It is used in
 * feedback delay networks (FDNs) and other DSP structures. The line can read
 * delayed samples, write new samples, and process them in-place.
 */
class DelayLine
{
public:
    /**
     * @brief Constructs a DelayLine with given parameters.
     * @param M The maximum delay length in samples.
     * @param g Gain applied to the delayed output (usually between 0 and 1).
     * @param maxBlockSize Maximum block size for internal processing buffers.
     */
    DelayLine(int M, float g, int maxBlockSize);

    /**
     * @brief Default constructor.
     *
     * Creates an uninitialized delay line. You must configure it before use.
     */
    DelayLine() = default;

    /**
     * @brief Destructor.
     */
    ~DelayLine() = default;

    // Copy operations are deleted to prevent accidental deep copies
    DelayLine(const DelayLine&) = delete;
    DelayLine& operator=(const DelayLine&) = delete;

    // Move operations are defaulted (safe for internal buffers)
    DelayLine(DelayLine&&) noexcept = default;
    DelayLine& operator=(DelayLine&&) noexcept = default;

    // --- Block-based API ---

    /**
     * @brief Reads a block of samples with a specified delay.
     * @param tau The delay in samples.
     * @param blockSize Number of samples to read.
     * @return Pointer to the beginning of the block.
     */
    float* readBlock(const int tau, const int blockSize) const;

    /**
     * @brief Reads a block of samples from the current delay line state.
     * @param blockSize Number of samples to read.
     * @return Pointer to the beginning of the block.
     */
    float* readBlock(const int blockSize) const;

    /**
     * @brief Writes a block of samples into the delay line.
     * @param input Pointer to the input samples.
     * @param blockSize Number of samples to write.
     */
    void writeBlock(const float* input, const int blockSize);

    /**
     * @brief Processes a block of samples in-place through the delay line.
     * @param block Pointer to the block of samples.
     * @param blockSize Number of samples in the block.
     */
    void processBlock(const float* block, const int blockSize);

    // --- Sample-based API ---

    /**
     * @brief Reads a delayed sample.
     * @param tau The delay in samples.
     * @return Delayed sample value.
     */
    float readSample(const int tau) const;

    /**
     * @brief Reads the most recent sample from the delay line.
     * @return The last written sample.
     */
    float readSample() const;

    /**
     * @brief Writes a single sample into the delay line.
     * @param input The sample to write.
     */
    void writeSample(const float input);

    /**
     * @brief Processes a single sample in-place through the delay line.
     * @param input The sample to process.
     */
    void processSample(const float& input);

private:
    int M = 0;                  /**< Maximum delay length in samples */
    float g = 0.0f;             /**< Gain applied to delayed output */

    std::vector<float> buffer;  /**< Circular buffer storing delayed samples */
    int bufferSize = 0;         /**< Total size of the buffer */
    int write = 0;              /**< Current write index in the buffer */

    // Allow FDN to access private members directly
    friend class FDN;
};
