#include "PluginProcessor.h"
#include "PluginEditor.h"

UmbraAudioProcessorEditor::UmbraAudioProcessorEditor(UmbraAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(800, 300);

    auto makeKnob = [this](juce::Slider& knob, juce::Label& label, const juce::String& labelText,
        double min = 0.0, double max = 1.0, bool isFrequency = false)
        {
            knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            knob.setRange(min, max, 0.01);
            knob.setValue(min);

            if (isFrequency)
                knob.setSkewFactorFromMidPoint(1000.0); // perceptual midpoint at 1kHz

            knob.setLookAndFeel(&customLookAndFeel);

            // Changed from white to velvet
            knob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour::fromFloatRGBA(0.5f, 0.0f, 0.25f, 1.0f));
            knob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour::fromFloatRGBA(0.5f, 0.0f, 0.25f, 1.0f));
            addAndMakeVisible(knob);

            label.setText(labelText, juce::dontSendNotification);
            label.setJustificationType(juce::Justification::centredTop);
            // Label text color changed to velvet
            label.setColour(juce::Label::textColourId, juce::Colour::fromFloatRGBA(0.5f, 0.0f, 0.25f, 1.0f));
            label.setFont(juce::Font("Courier New", 14.0f, juce::Font::bold));
            label.attachToComponent(&knob, false);
            addAndMakeVisible(label);
        };

    // Create knobs and labels
    makeKnob(mixKnob, mixLabel, "Mix");
    makeKnob(roomSizeKnob, roomSizeLabel, "Room Size");
    makeKnob(dampeningKnob, dampeningLabel, "Dampening");
    makeKnob(stereoWidthKnob, stereoWidthLabel, "Stereo Width");
    makeKnob(initialDelayKnob, initialDelayLabel, "Initial Delay");

    // Here's the important part:
    makeKnob(lowPassKnob, lowPassLabel, "Low Pass", 20.0, 20000.0, true);
    makeKnob(highPassKnob, highPassLabel, "High Pass", 20.0, 20000.0, true);

    // --- ATTACHMENTS ---
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "mix", mixKnob);

    roomSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "roomSize", roomSizeKnob);

    dampeningAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "dampening", dampeningKnob);

    stereoWidthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "stereoWidth", stereoWidthKnob);

    initialDelayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "initialDelay", initialDelayKnob);

    lowPassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "lowPass", lowPassKnob);

    highPassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "highPass", highPassKnob);
    // --- END ATTACHMENTS ---

    addAndMakeVisible(spectrogram3D);

    // Pass pointer to FFTProcessor so Spectrogram3DComponent can access FFT data
    spectrogram3D.setFFTProcessor(&audioProcessor.fftProcessor);

    // Removed startTimer here since timer is now in Spectrogram3DComponent
}

UmbraAudioProcessorEditor::~UmbraAudioProcessorEditor()
{
    mixKnob.setLookAndFeel(nullptr);
    roomSizeKnob.setLookAndFeel(nullptr);
    dampeningKnob.setLookAndFeel(nullptr);
    stereoWidthKnob.setLookAndFeel(nullptr);
    initialDelayKnob.setLookAndFeel(nullptr);
    lowPassKnob.setLookAndFeel(nullptr);
    highPassKnob.setLookAndFeel(nullptr);
}

void UmbraAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void UmbraAudioProcessorEditor::resized()
{
    const int totalKnobs = 7;
    const int spacing = 10;
    const int knobHeight = 70;
    const int knobTopMargin = 40;

    int totalWidth = getWidth();
    int totalHeight = getHeight();
    int totalSpacing = spacing * (totalKnobs - 1);
    int knobWidth = (totalWidth - totalSpacing) / totalKnobs;

    for (int i = 0; i < totalKnobs; ++i)
    {
        int x = i * (knobWidth + spacing);
        int y = knobTopMargin;
        switch (i)
        {
        case 0: mixKnob.setBounds(x, y, knobWidth, knobHeight); break;
        case 1: roomSizeKnob.setBounds(x, y, knobWidth, knobHeight); break;
        case 2: dampeningKnob.setBounds(x, y, knobWidth, knobHeight); break;
        case 3: stereoWidthKnob.setBounds(x, y, knobWidth, knobHeight); break;
        case 4: initialDelayKnob.setBounds(x, y, knobWidth, knobHeight); break;
        case 5: lowPassKnob.setBounds(x, y, knobWidth, knobHeight); break;
        case 6: highPassKnob.setBounds(x, y, knobWidth, knobHeight); break;
        }
    }

    const int spectrogramHeight = 165;
    int spectrogramWidth = totalWidth;
    int spectrogramX = juce::jmax(0, (totalWidth - spectrogramWidth) / 2);
    int spectrogramY = totalHeight - spectrogramHeight;

    spectrogram3D.setBounds(spectrogramX, spectrogramY, spectrogramWidth, spectrogramHeight);
}
