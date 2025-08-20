#pragma once

// Standard library
#include <vector>
#include <stdexcept>

// JUCE
#include <JuceHeader.h>

/**
 * @class Hadamard
 * @brief Provides static functions for performing a Hadamard transform.
 *
 * The Hadamard transform is an orthogonal, fast transform similar to the
 * Fourier transform but using only addition and subtraction. This class
 * operates on audio buffers or standard float vectors.
 *
 * This class is non-instantiable; all functions are static.
 */
class Hadamard
{
public:
    /**
     * @brief Delete default constructor to prevent instantiation.
     */
    Hadamard() = delete;

    /**
     * @brief Performs a Hadamard transform on a JUCE audio buffer.
     * @param buffer The audio buffer to transform. Transformation is applied in-place.
     *
     * Only the first channel is processed; multi-channel usage must be handled externally.
     */
    static void process(juce::AudioBuffer<float>& buffer);

    /**
     * @brief Performs a Hadamard transform on a standard float vector.
     * @param frame The vector to transform. Transformation is applied in-place.
     *
     * Vector size must be a power of two.
     */
    static void process(std::vector<float>& frame);
};
