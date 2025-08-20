#pragma once

// JUCE
#include <JuceHeader.h>

/**
 * @class CustomLookAndFeel
 * @brief A custom JUCE LookAndFeel for rotary sliders with a velvet-colored knob
 *        and subtle static noise overlay for a vintage aesthetic.
 *
 * This LookAndFeel overrides the default rotary slider appearance, drawing
 * a circular knob with a fill proportional to the slider position and
 * an overlay of random noise pixels to give a tactile "glow" effect.
 */
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    /** @brief Default constructor. */
    CustomLookAndFeel() = default;

    /** @brief Destructor. */
    ~CustomLookAndFeel() override = default;

    /** @brief Copy constructor deleted (JUCE LookAndFeel objects are non-copyable). */
    CustomLookAndFeel(const CustomLookAndFeel&) = delete;

    /** @brief Copy assignment deleted (JUCE LookAndFeel objects are non-copyable). */
    CustomLookAndFeel& operator=(const CustomLookAndFeel&) = delete;

    /** @brief Defaulted move constructor (safe to move). */
    CustomLookAndFeel(CustomLookAndFeel&&) noexcept = default;

    /** @brief Defaulted move assignment (safe to move). */
    CustomLookAndFeel& operator=(CustomLookAndFeel&&) noexcept = default;

    /**
     * @brief Draw a custom rotary slider.
     * @param g The Graphics context for drawing.
     * @param x Top-left x coordinate of the slider bounding box.
     * @param y Top-left y coordinate of the slider bounding box.
     * @param width Width of the bounding box.
     * @param height Height of the bounding box.
     * @param sliderPosProportional Normalized slider position [0..1].
     * @param rotaryStartAngle Ignored in this implementation.
     * @param rotaryEndAngle Ignored in this implementation.
     * @param slider Reference to the JUCE Slider (not used here).
     *
     * Draws a circular velvet-colored knob, a filled arc corresponding
     * to the slider's value, and overlays subtle static noise.
     */
    void drawRotarySlider(juce::Graphics& g,
        int x,
        int y,
        int width,
        int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider& slider) override;

private:
    /**
     * @brief Draws subtle random static noise within a given area.
     * @param g The Graphics context for drawing.
     * @param area The rectangular area to apply noise.
     *
     * Each pixel has a small probability of being a faint white or black
     * semi-transparent noise dot, giving the knob a textured "glow" effect.
     */
    void drawStaticNoise(juce::Graphics& g, const juce::Rectangle<float>& area);
};
