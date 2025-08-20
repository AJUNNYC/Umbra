#pragma once

// Standard library
#include <vector>

// JUCE
#include <JuceHeader.h>

/**
 * @class FFTProcessor
 * @brief Real-time FFT processor for audio buffers.
 *
 * This class accumulates incoming audio samples into a buffer,
 * performs a forward FFT once enough samples are collected,
 * and stores the magnitude spectrum for retrieval.
 * Thread-safe access is provided using a CriticalSection.
 */
class FFTProcessor
{
public:
    /**
     * @brief Constructs the FFTProcessor.
     * @param order The FFT order (fftSize = 2^order). Default is 10 (1024 samples).
     */
    explicit FFTProcessor(int order = 10);

    /** @brief Destructor. */
    ~FFTProcessor() = default;

    /** @brief Copy constructor deleted to prevent accidental buffer duplication. */
    FFTProcessor(const FFTProcessor&) = delete;
    /** @brief Copy assignment deleted to prevent accidental buffer duplication. */
    FFTProcessor& operator=(const FFTProcessor&) = delete;

    /** @brief Move constructor is defaulted (safe for member buffers). */
    FFTProcessor(FFTProcessor&&) noexcept = default;
    /** @brief Move assignment is defaulted (safe for member buffers). */
    FFTProcessor& operator=(FFTProcessor&&) noexcept = default;

    /**
     * @brief Prepare the FFTProcessor for new processing.
     *
     * Clears the internal FFT buffer and resets the write position.
     */
    void prepare();

    /**
     * @brief Push a block of audio samples into the FFT buffer.
     * @param buffer Mono audio buffer containing samples to process.
     *
     * Once enough samples are collected, a forward FFT is performed
     * and the magnitude spectrum becomes available.
     */
    void pushSamples(const juce::AudioBuffer<float>& buffer);

    /**
     * @brief Retrieve the most recent FFT magnitude data.
     * @return A vector of floats representing magnitude spectrum (size fftSize / 2).
     *
     * Thread-safe access ensured by internal locking.
     */
    std::vector<float> getFFTData();

    /**
     * @brief Check if a new FFT block is ready and reset the ready flag.
     * @return true if a new FFT block was computed since last call.
     *
     * Thread-safe access ensured by internal locking.
     */
    bool getAndResetFFTReadyFlag();

    /**
     * @brief Get the FFT size (number of samples per FFT block).
     * @return FFT size (power of 2)
     */
    int getFFTSize() const { return fftSize; }

private:
    int fftSize = 0;                             ///< Number of samples per FFT block (2^order)
    juce::dsp::FFT forwardFFT;                   ///< JUCE FFT object for forward transform
    juce::AudioBuffer<float> fftBuffer;          ///< Circular buffer for accumulating input samples
    int fftBufferWritePos = 0;                   ///< Current write position in fftBuffer

    std::vector<float> fftData;                  ///< Stores the computed magnitude spectrum
    bool nextFFTBlockReady = false;              ///< Flag indicating that new FFT data is available
    juce::CriticalSection fftDataLock;           ///< Protects fftData and nextFFTBlockReady for thread safety
};
