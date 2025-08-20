#include "Spectrogram3DComponent.h"
#include "FFTProcessor.h"
#include <algorithm>
#include <cmath>

using namespace juce;
using namespace gl;

/**
 * @brief Constructs the 3D spectrogram component.
 *
 * Initializes the history buffer with zeros, sets up the OpenGL context,
 * and starts a JUCE timer to update at 30 Hz.
 */
Spectrogram3DComponent::Spectrogram3DComponent()
{
    setOpaque(false); // Allow transparency for overlay effects

    // Initialize circular buffer for spectrum history
    history.resize(maxHistoryLength, std::vector<float>(numFrequencyBins, 0.0f));

    // Set up OpenGL context
    openGLContext.setRenderer(this);
    openGLContext.attachTo(*this);
    openGLContext.setContinuousRepainting(false);

    // Start timer for periodic FFT updates
    startTimerHz(30);
}

/**
 * @brief Destructor detaches the OpenGL context.
 *
 * Ensures proper cleanup of OpenGL resources.
 */
Spectrogram3DComponent::~Spectrogram3DComponent()
{
    openGLContext.detach();
}

/**
 * @brief Sets the FFT processor as the data source.
 *
 * Also updates the number of frequency bins based on the FFT size.
 *
 * @param processor Pointer to an FFTProcessor instance
 */
void Spectrogram3DComponent::setFFTProcessor(FFTProcessor* processor)
{
    fftProcessor = processor;
    numFrequencyBins = fftProcessor->getFFTSize() / 2;
}

/**
 * @brief Push a new spectrum frame into the circular history buffer.
 *
 * Thread-safe with a mutex. If the size of the input vector does not
 * match the expected number of frequency bins, the frame is ignored.
 *
 * @param magnitudes Vector of FFT magnitudes for the current frame
 */
void Spectrogram3DComponent::pushSpectrumData(const std::vector<float>& magnitudes)
{
    std::lock_guard<std::mutex> lock(dataMutex);

    if (magnitudes.size() != static_cast<size_t>(numFrequencyBins))
        return;

    history[writeIndex] = magnitudes;
    writeIndex = (writeIndex + 1) % maxHistoryLength;
}

/**
 * @brief JUCE timer callback for periodic updates.
 *
 * If a new FFT block is ready, fetches the data from the FFT processor
 * and pushes it into the history buffer. If no signal is detected,
 * pushes a zeroed frame to maintain the history.
 */
void Spectrogram3DComponent::timerCallback()
{
    if (fftProcessor && fftProcessor->getAndResetFFTReadyFlag())
    {
        const auto fftCopy = fftProcessor->getFFTData();
        bool hasSignal = std::any_of(fftCopy.begin(), fftCopy.end(),
            [](float v) { return v > 0.0001f; });
        pushSpectrumData(hasSignal ? fftCopy : std::vector<float>(fftCopy.size(), 0.0f));
    }
    else
    {
        pushSpectrumData(std::vector<float>(numFrequencyBins, 0.0f));
    }

    // Trigger OpenGL repaint
    openGLContext.triggerRepaint();
}

/**
 * @brief OpenGL initialization callback.
 *
 * Enables blending, sets the clear color, and creates a vertex buffer object (VBO)
 * for efficient rendering of spectrum lines.
 */
void Spectrogram3DComponent::newOpenGLContextCreated()
{
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageControl(
        GL_DEBUG_SOURCE_API,
        GL_DEBUG_TYPE_OTHER,
        GL_DEBUG_SEVERITY_NOTIFICATION,
        0,
        nullptr,
        GL_FALSE
    );
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Transparent clear color

    glGenBuffers(1, &vbo);
}

/**
 * @brief OpenGL cleanup callback.
 *
 * Deletes the VBO to free GPU resources.
 */
void Spectrogram3DComponent::openGLContextClosing()
{
    glDeleteBuffers(1, &vbo);
}

/**
 * @brief Render the 3D spectrogram using OpenGL.
 *
 * This function performs the following steps:
 * 1. Clears color and depth buffers
 * 2. Sets up camera projection and view
 * 3. Normalizes FFT magnitudes to dB and maps them to a 3D box
 * 4. Generates vertices for a logarithmic frequency wireframe
 * 5. Uploads vertex data to a VBO and renders lines
 */
void Spectrogram3DComponent::renderOpenGL()
{
    // Clear frame and enable depth test
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Setup viewport
    auto bounds = getLocalBounds();
    glViewport(0, 0, bounds.getWidth(), bounds.getHeight());

    // Setup projection matrix (perspective)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)bounds.getWidth() / bounds.getHeight(), 0.1, 1000.0);

    // Setup modelview matrix (camera)
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        0.0, 250.0, 250.0, // Camera position
        0.0, 0.0, 0.0,     // Look-at target
        0.0, 1.0, 0.0      // Up vector
    );

    // 3D box dimensions
    const float boxWidth = 750.0f;
    const float boxHeight = 100.0f;
    const float boxDepth = 200.0f;

    // Velvet color for wireframe
    const float velvetR = 0.35f, velvetG = 0.05f, velvetB = 0.12f;

    // Copy history into a local buffer for thread safety
    std::vector<std::vector<float>> localHistory;
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        for (int i = 0; i < maxHistoryLength; ++i)
        {
            int idx = (writeIndex + maxHistoryLength - 1 - i) % maxHistoryLength;
            localHistory.push_back(history[idx]);
        }
    }

    if (localHistory.empty())
        return;

    const float zStep = boxDepth / static_cast<float>(localHistory.size());

    // Lambda to normalize magnitude to [0,1] using dB conversion
    auto normalizedMagnitude = [&](const std::vector<float>& frame, int bin) -> float {
        if (bin < 0 || bin >= static_cast<int>(frame.size()))
            return 0.0f;

        float magnitude = juce::jlimit(0.0f, 1.0f, frame[bin]);
        float db = juce::Decibels::gainToDecibels(magnitude, -60.0f);
        float normDb = juce::jmap(db, -60.0f, 0.0f, 0.0f, 1.0f);
        return juce::jlimit(0.0f, 1.0f, normDb);
        };

    // Visual resolution along frequency axis
    constexpr int numVisualBins = 25;
    float minLog = log10f(1.0f);
    float maxLog = log10f(static_cast<float>(numFrequencyBins));

    // Set OpenGL state for line rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glShadeModel(GL_FLAT);
    glLineWidth(1.2f);
    glColor4f(velvetR, velvetG, velvetB, 0.7f);

    // Accumulate vertex positions for all wireframe edges
    std::vector<GLfloat> edgeVertices;

    for (size_t z = 0; z + 1 < localHistory.size(); ++z)
    {
        const auto& frameA = localHistory[z];
        const auto& frameB = localHistory[z + 1];

        const float zA = boxDepth / 2.0f - z * zStep;
        const float zB = boxDepth / 2.0f - (z + 1) * zStep;

        for (int i = 0; i < numVisualBins; ++i)
        {
            float logT = juce::jmap(static_cast<float>(i), 0.0f, static_cast<float>(numVisualBins - 1), minLog, maxLog);
            int bin = juce::jlimit(0, numFrequencyBins - 1, static_cast<int>(std::floor(powf(10.0f, logT))));

            float magA = normalizedMagnitude(frameA, bin);
            float magB = normalizedMagnitude(frameB, bin);

            if (magA < 0.001f && magB < 0.001f)
                continue;

            float xPos = juce::jmap(logT, minLog, maxLog, -boxWidth / 2.0f, boxWidth / 2.0f);
            float yA = juce::jmap(magA, 0.0f, 1.0f, -boxHeight / 2.0f, boxHeight / 2.0f);
            float yB = juce::jmap(magB, 0.0f, 1.0f, -boxHeight / 2.0f, boxHeight / 2.0f);

            // Vertical line between frames
            edgeVertices.push_back(xPos); edgeVertices.push_back(yA); edgeVertices.push_back(zA);
            edgeVertices.push_back(xPos); edgeVertices.push_back(yB); edgeVertices.push_back(zB);

            // Horizontal connection to next bin
            if (i + 1 < numVisualBins)
            {
                float nextLogT = juce::jmap(static_cast<float>(i + 1), 0.0f, static_cast<float>(numVisualBins - 1), minLog, maxLog);
                int nextBin = juce::jlimit(0, numFrequencyBins - 1, static_cast<int>(std::round(powf(10.0f, nextLogT))));
                float magA2 = normalizedMagnitude(frameA, nextBin);
                if (magA2 > 0.001f)
                {
                    float xNext = juce::jmap(nextLogT, minLog, maxLog, -boxWidth / 2.0f, boxWidth / 2.0f);
                    float yA2 = juce::jmap(magA2, 0.0f, 1.0f, -boxHeight / 2.0f, boxHeight / 2.0f);

                    edgeVertices.push_back(xPos);  edgeVertices.push_back(yA);  edgeVertices.push_back(zA);
                    edgeVertices.push_back(xNext); edgeVertices.push_back(yA2); edgeVertices.push_back(zA);
                }
            }
        }
    }

    // Upload vertex data and draw lines using VBO
    if (!edgeVertices.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, edgeVertices.size() * sizeof(GLfloat), edgeVertices.data(), GL_DYNAMIC_DRAW);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, nullptr);

        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(edgeVertices.size() / 3));

        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Reset OpenGL state
    glDisable(GL_BLEND);
    glLineWidth(1.0f);
    glDisable(GL_DEPTH_TEST);
}
