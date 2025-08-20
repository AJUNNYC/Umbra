#include "FDN.h"
#include <random>
#include <cmath>

/**
 * @brief Constructs a Feedback Delay Network with randomized delay lines and feedback gains.
 *
 * Each delay line length is jittered randomly around the base length `m` to decorrelate echoes.
 * Feedback gains are randomly assigned to provide a natural-sounding reverberation.
 * Low-pass damping filters are initialized with default parameters (will be updated in process()).
 *
 * @param N Number of delay lines (and typically audio channels).
 * @param m Base delay length in samples.
 * @param blockSize Maximum block size for internal buffers.
 */
FDN::FDN(const int& N, const int& m, int blockSize) : N(N)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> jitter(1.0f, 2.0f); // Delay jitter factor
    std::uniform_real_distribution<float> gain(0.8f, 0.9f);   // Feedback gains

    z.push_back(std::make_unique<DelayLine>(0, 0.0f, blockSize)); // Dummy first delay

    M.resize(N);
    for (int i = 1; i < N; ++i)
    {
        // Randomize delay lengths
        M[i] = static_cast<int>(std::round(m * jitter(gen)));
        z.push_back(std::make_unique<DelayLine>(2 * M[i], 0.0f, blockSize));
    }

    g.resize(N);
    for (int i = 0; i < N; ++i)
        g[i] = gain(gen);

    H.resize(N);
    for (auto& filter : H)
        filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(44100.0, 8000.0);
}

/**
 * @brief Processes an audio buffer through the FDN.
 *
 * Steps:
 * 1. Read each delay line at the scaled delay position (M[ch] * roomSize).
 * 2. Mix all delay line outputs using a Hadamard transform for energy redistribution.
 * 3. Apply damping filter (low-pass) and feedback gain.
 * 4. Add input signal to the feedback signal and write back into the delay lines.
 * 5. Replace the input buffer with the processed wet signal.
 *
 * @param buffer Audio buffer to process in-place.
 * @param dampening Low-pass cutoff frequency (Hz) for damping filters.
 * @param fs Sample rate (Hz).
 * @param roomSize Scaling factor affecting effective delay read positions.
 */
void FDN::process(juce::AudioBuffer<float>& buffer, float dampening, double fs, float roomSize)
{
    const int numSamples = buffer.getNumSamples();
    const int N = static_cast<int>(z.size()); // Number of delay lines

    std::vector<float> inputFrame(N, 0.0f);
    std::vector<float> outputFrame(N, 0.0f);

    // Update low-pass damping filter coefficients for each delay line
    for (int ch = 0; ch < N; ++ch)
        H[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(fs, dampening);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Step 1: Read current input samples into frame
        for (int ch = 0; ch < N; ++ch)
            inputFrame[ch] = buffer.getSample(ch, sample);

        // Step 2: Read delayed samples for feedback
        for (int ch = 0; ch < N; ++ch)
            outputFrame[ch] = z[ch]->readSample(static_cast<int>(M[ch] * roomSize));

        // Step 3: Write processed wet signal to output buffer
        for (int ch = 0; ch < N; ++ch)
            buffer.setSample(ch, sample, outputFrame[ch]);

        // Step 4: Mix delay line outputs using Hadamard transform
        Hadamard::process(outputFrame);

        // Step 5: Apply damping filter and feedback gain, then add input
        for (int ch = 0; ch < N; ++ch)
            outputFrame[ch] = inputFrame[ch] + g[ch] * H[ch].processSample(outputFrame[ch]);

        // Step 6: Write processed samples back into delay lines
        for (int ch = 0; ch < N; ++ch)
            z[ch]->writeSample(outputFrame[ch]);
    }
}
