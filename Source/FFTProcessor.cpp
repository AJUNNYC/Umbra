#include "FFTProcessor.h"

/**
 * @brief Constructs the FFTProcessor with a given FFT order.
 * @param order FFT order (fftSize = 2^order). Default 10 -> 1024 samples.
 */
FFTProcessor::FFTProcessor(int order)
    : fftSize(1 << order), forwardFFT(order)
{
    // Allocate mono buffer to accumulate incoming samples
    fftBuffer.setSize(1, fftSize);
    fftBuffer.clear();

    // Allocate magnitude spectrum buffer (half size of FFT)
    fftData.resize(fftSize / 2, 0.0f);
}

/**
 * @brief Clears the internal FFT buffer and resets state.
 */
void FFTProcessor::prepare()
{
    fftBuffer.clear();
    fftBufferWritePos = 0;
    nextFFTBlockReady = false;
}

/**
 * @brief Push a block of samples into the FFT processor.
 *
 * Accumulates samples until fftSize is reached, then performs a
 * forward FFT and stores the magnitude spectrum.
 * Thread-safe storage of fftData is ensured via CriticalSection.
 *
 * @param buffer Mono audio buffer containing new samples.
 */
void FFTProcessor::pushSamples(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        // Write sample into circular buffer
        fftBuffer.setSample(0, fftBufferWritePos, buffer.getSample(0, i));
        ++fftBufferWritePos;

        // If buffer is full, compute FFT
        if (fftBufferWritePos >= fftSize)
        {
            fftBufferWritePos = 0;

            // Allocate temporary FFT array (real+imag interleaved)
            juce::HeapBlock<float> fftDataBlock(fftSize * 2);
            std::fill(fftDataBlock.get(), fftDataBlock.get() + fftSize * 2, 0.0f);

            // Copy input samples to real part
            for (int j = 0; j < fftSize; ++j)
                fftDataBlock[j * 2] = fftBuffer.getSample(0, j);

            // Perform forward FFT (magnitude only)
            forwardFFT.performFrequencyOnlyForwardTransform(fftDataBlock);

            // Copy magnitude spectrum into thread-safe buffer
            {
                const juce::ScopedLock lock(fftDataLock);
                for (int k = 0; k < fftSize / 2; ++k)
                    fftData[k] = fftDataBlock[k];
                nextFFTBlockReady = true;
            }
        }
    }
}

/**
 * @brief Retrieve the most recent FFT magnitude spectrum.
 * @return Vector of float magnitudes (size fftSize / 2).
 */
std::vector<float> FFTProcessor::getFFTData()
{
    const juce::ScopedLock lock(fftDataLock);
    return fftData;
}

/**
 * @brief Check if a new FFT block is ready, then reset the flag.
 * @return True if new FFT data was computed since last call.
 */
bool FFTProcessor::getAndResetFFTReadyFlag()
{
    const juce::ScopedLock lock(fftDataLock);
    bool ready = nextFFTBlockReady;
    nextFFTBlockReady = false;
    return ready;
}
