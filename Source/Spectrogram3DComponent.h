#pragma once

// Standard library
#include <vector>
#include <mutex>

// JUCE
#include <JuceHeader.h>

// Platform-specific OpenGL headers
#if JUCE_WINDOWS
#include <windows.h>    // Must come before OpenGL headers on Windows
#include <GL/gl.h>      // Core OpenGL
#include <GL/glu.h>     // GLU for gluLookAt, gluPerspective, etc.
#pragma comment(lib, "glu32.lib")
#endif

// Forward declarations
class FFTProcessor;

/**
 * @class Spectrogram3DComponent
 * @brief A 3D spectrogram visualizer using OpenGL for high-performance rendering.
 *
 * This component maintains a rolling history of FFT magnitudes and renders
 * them as a 3D wireframe box, with logarithmically spaced frequency bins
 * and a "velvet" color scheme. It is optimized for real-time updates
 * using OpenGL vertex buffers.
 */
class Spectrogram3DComponent : public juce::Component,
    private juce::Timer,
    private juce::OpenGLRenderer
{
public:
    /** @brief Constructs the 3D spectrogram and attaches an OpenGL context. */
    Spectrogram3DComponent();

    /** @brief Destructor detaches OpenGL context and cleans up buffers. */
    ~Spectrogram3DComponent() override;

    /**
     * @brief Push a new spectrum frame into the history.
     * @param magnitudes Vector of FFT magnitudes for the current frame.
     *
     * The vector size must match the number of frequency bins.
     */
    void pushSpectrumData(const std::vector<float>& magnitudes);

    /**
     * @brief Set the FFT processor to read spectrum data from.
     * @param processor Pointer to an FFTProcessor instance.
     */
    void setFFTProcessor(FFTProcessor* processor);

private:
    // --- JUCE Timer callback ---
    void timerCallback() override;

    // --- OpenGLRenderer overrides ---
    void newOpenGLContextCreated() override;  ///< Called when OpenGL context is created.
    void openGLContextClosing() override;    ///< Called before OpenGL context is destroyed.
    void renderOpenGL() override;            ///< Called each frame to render the 3D spectrogram.

    // --- Spectrum history ---
    std::vector<std::vector<float>> history; ///< Circular buffer of spectrum frames.
    std::mutex dataMutex;                     ///< Protects history access across threads.
    int writeIndex = 0;                       ///< Current write position in the circular buffer.
    int numFrequencyBins = 2048;              ///< Number of FFT bins stored per frame.
    int maxHistoryLength = 50;                ///< Number of frames stored in history.

    // --- OpenGL ---
    juce::OpenGLContext openGLContext;        ///< OpenGL context for rendering.
    GLuint vbo = 0;                           ///< Vertex buffer object for lines.

    // --- FFT processor ---
    FFTProcessor* fftProcessor = nullptr;     ///< Optional source of FFT data.

    // Prevent copying and enable leak detection
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Spectrogram3DComponent)
};
