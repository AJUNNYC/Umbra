#include "CustomLookAndFeel.h"
#include <random>

/**
 * @brief Draw a custom rotary slider with velvet-colored knob and static noise overlay.
 *
 * This function overrides the JUCE default LookAndFeel to produce a circular knob
 * with a velvet fill and a subtle “glow” effect using random static noise.
 *
 * @param g Graphics context for drawing.
 * @param x Top-left x coordinate of the slider bounding box.
 * @param y Top-left y coordinate of the slider bounding box.
 * @param width Width of the bounding box.
 * @param height Height of the bounding box.
 * @param sliderPosProportional Normalized slider position [0..1].
 * @param rotaryStartAngle Ignored here.
 * @param rotaryEndAngle Ignored here.
 * @param slider Reference to the JUCE Slider object (unused).
 */
void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
    float sliderPosProportional, float /*rotaryStartAngle*/,
    float /*rotaryEndAngle*/, juce::Slider& /*slider*/)
{
    // Use low-quality resampling for performance; pixel-level accuracy is not critical
    g.setImageResamplingQuality(juce::Graphics::ResamplingQuality::lowResamplingQuality);

    // Define the drawing bounds and knob center
    const auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height);
    const auto center = bounds.getCentre();
    const float radius = juce::jmin(width, height) / 2.0f - 3.0f;  // Padding for outline
    const float outlineThickness = 1.0f;

    // Velvet knob color
    juce::Colour velvet = juce::Colour::fromFloatRGBA(0.35f, 0.05f, 0.12f, 1.0f);

    // --- Draw the circular outline ---
    g.setColour(velvet);
    g.drawEllipse(center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f, outlineThickness);

    // --- Draw the fill arc proportional to the slider value ---
    float startAngle = juce::MathConstants<float>::pi;  // Start at left side of knob
    float endAngle = startAngle + sliderPosProportional * juce::MathConstants<float>::twoPi;  // Full rotation scaled

    if (sliderPosProportional > 0.0f)
    {
        juce::Path pie;
        pie.startNewSubPath(center);  // Move to center for pie shape
        pie.addCentredArc(center.x, center.y,
            radius - outlineThickness / 2,  // Reduce radius slightly to stay inside outline
            radius - outlineThickness / 2,
            0.0f, startAngle, endAngle, true);  // Arc from start to end
        pie.lineTo(center);  // Close path back to center

        g.setColour(velvet);
        g.fillPath(pie);
    }

    // --- Overlay subtle static noise for tactile/glow effect ---
    drawStaticNoise(g, bounds);
}

/**
 * @brief Draw subtle random static noise within the knob area.
 *
 * Creates an off-screen image and sets random pixels to faint white or black
 * with low opacity to simulate texture and visual “glow”.
 *
 * @param g Graphics context for drawing.
 * @param area Rectangular area corresponding to the knob bounds.
 */
void CustomLookAndFeel::drawStaticNoise(juce::Graphics& g, const juce::Rectangle<float>& area)
{
    // Random number generator
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    const int pixelSize = 2;  // Draw every 2 pixels for subtlety
    juce::Image noiseImage(juce::Image::ARGB,
        static_cast<int>(area.getWidth()),
        static_cast<int>(area.getHeight()), true);

    // Iterate through pixels and randomly assign faint noise
    for (int x = 0; x < noiseImage.getWidth(); x += pixelSize)
    {
        for (int y = 0; y < noiseImage.getHeight(); y += pixelSize)
        {
            if (dist(rng) < 0.03f) // 3% chance of noise pixel
            {
                // Randomly choose faint white or black with 10% opacity
                juce::Colour c = (dist(rng) < 0.5f) ?
                    juce::Colours::white.withAlpha(0.1f) :
                    juce::Colours::black.withAlpha(0.1f);

                noiseImage.setPixelAt(x, y, c);
            }
        }
    }

    // Draw the noise image over the knob
    g.drawImageAt(noiseImage, static_cast<int>(area.getX()), static_cast<int>(area.getY()));
}
