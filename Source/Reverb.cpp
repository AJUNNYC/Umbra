#include "Reverb.h"

/**
 * @brief Constructs a Reverb with given sample rate and block size.
 *
 * Initializes:
 * - Three diffusers (DVN-based) with preconfigured pulse counts and sizes.
 * - Two FDNs for late reverb.
 * - Initial delay lines (pre-delay).
 * - Low/high-pass filters.
 *
 * @param fs Sample rate in Hz.
 * @param blockSize Maximum audio block size.
 */
Reverb::Reverb(float fs, int blockSize)
    : fs(fs), blockSize(blockSize),
    d1(8, 200, 2000, blockSize, fs),
    d2(8, 200, 2000, blockSize, fs),
    d3(8, 200, 2000, blockSize, fs),
    fdn1(8, static_cast<int>(0.1f * fs), blockSize),
    fdn2(8, static_cast<int>(0.1f * fs), blockSize)
{
    // Pre-delay lines for first 2 channels
    z.resize(2);
    for (int ch = 0; ch < 2; ++ch)
        z[ch] = DelayLine(static_cast<int>(0.1f * fs), 1.0f, blockSize);


    // Initialize per-channel filters 
    lowPassFilters.resize(2); 
    highPassFilters.resize(2); 
    
    for (int ch = 0; ch < 2; ++ch)
    {
        lowPassFilters[ch].reset();
        highPassFilters[ch].reset();

        lowPassFilters[ch].setCoefficients(
            juce::IIRCoefficients::makeLowPass(fs, 20000.0)
        );

        highPassFilters[ch].setCoefficients(
            juce::IIRCoefficients::makeHighPass(fs, 20.0)
        );
    }

    // Store the current cutoff values so they can be updated dynamically later
    previousLowPass = 20000.0f;
    previousHighPass = 20.0f;

}

/**
 * @brief Processes an audio buffer with the full reverb chain.
 *
 * Steps:
 * 1. Resize initial delay lines to match number of channels if needed.
 * 2. Update low/high-pass filters if their cutoff frequencies changed.
 * 3. Store a copy of the dry buffer.
 * 4. Apply high-pass and low-pass filtering in-place.
 * 5. Apply initial pre-delay per channel using DelayLine.
 * 6. Upscale buffer to 4 channels if original channels < 4.
 * 7. Apply three diffuser stages and two FDN stages.
 * 8. Apply stereo width adjustment (Mid/Side) if stereo.
 * 9. Mix dry and wet signals in-place using inline references.
 *
 * @param buffer Audio buffer to process.
 * @param mix Dry/wet mix (0.0 = dry, 1.0 = fully wet).
 * @param stereoWidth Stereo width factor (>1 widens stereo).
 * @param lowPass Low-pass cutoff (Hz).
 * @param highPass High-pass cutoff (Hz).
 * @param dampening FDN damping coefficient.
 * @param roomSize Scaling factor for FDN delay indices.
 * @param initialDelay Pre-delay in seconds applied before reverb.
 */
void Reverb::process(juce::AudioBuffer<float>& buffer,
    float mix,
    float stereoWidth,
    float lowPass,
    float highPass,
    float dampening,
    float roomSize,
    float initialDelay)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Resize pre-delay lines if number of channels changed
    if (z.size() != numChannels)
    {
        z.resize(numChannels);
        for (int ch = 0; ch < numChannels; ++ch)
            z[ch] = DelayLine(static_cast<int>(0.1f * fs), 1.0f, blockSize);
    }

    // Update filters if cutoff changed - update all channel instances
    if (lowPass != previousLowPass)
    {
        auto coeffs = juce::IIRCoefficients::makeLowPass(fs, lowPass);
        for (int ch = 0; ch < numChannels; ++ch)
            lowPassFilters[ch].setCoefficients(coeffs);
        previousLowPass = lowPass;
    }
    if (highPass != previousHighPass)
    {
        auto coeffs = juce::IIRCoefficients::makeHighPass(fs, highPass);
        for (int ch = 0; ch < numChannels; ++ch)
            highPassFilters[ch].setCoefficients(coeffs);
        previousHighPass = highPass;
    }


    // Store dry copy
    juce::AudioBuffer<float> dry;
    dry.makeCopyOf(buffer);

    // Apply filters with proper channel isolation
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* channelData = buffer.getWritePointer(ch);

        // Apply high-pass first, then low-pass
        highPassFilters[ch].processSamples(channelData, numSamples);
        lowPassFilters[ch].processSamples(channelData, numSamples);
    }


    // Apply pre-delay
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* bufferPtr = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float inSample = bufferPtr[i];
            bufferPtr[i] = z[ch].readSample(static_cast<int>(initialDelay * fs));
            z[ch].writeSample(inSample);
        }
    }

    // Upscale to 8 channels 
    buffer.setSize(8, numSamples, true, true, true);
    const float* lastChannelData = buffer.getReadPointer(numChannels - 1);
    for (int ch = numChannels; ch < 8; ++ch)
        buffer.copyFrom(ch, 0, lastChannelData, numSamples);

    // Apply reverb chain
    d1.process(buffer);
    fdn1.process(buffer, dampening, fs, roomSize);
    d2.process(buffer);
    fdn2.process(buffer, dampening, fs, roomSize);
    d3.process(buffer);

    // Stereo width adjustment (Mid/Side)
    if (numChannels == 2)
    {
        float* left = buffer.getWritePointer(0);
        float* right = buffer.getWritePointer(1);
        for (int i = 0; i < numSamples; ++i)
        {
            float mid = 0.5f * (left[i] + right[i]);
            float side = 0.5f * (left[i] - right[i]) * stereoWidth;
            left[i] = mid + side;
            right[i] = mid - side;
        }
    }

    // Inline wet reference
    auto& wet = buffer;

    // Mix dry and wet signals for first 2 channels
    for (int ch = 0; ch < 2; ++ch)
    {
        for (int i = 0; i < numSamples; ++i)
            wet.getWritePointer(ch)[i] = mix * wet.getWritePointer(ch)[i] +
            (1.0f - mix) * dry.getReadPointer(ch)[i];
    }
}
