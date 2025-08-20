#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"
#include "Spectrogram3DComponent.h"

class UmbraAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit UmbraAudioProcessorEditor(UmbraAudioProcessor&);
    ~UmbraAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    UmbraAudioProcessor& audioProcessor;

    juce::Slider mixKnob;
    juce::Label mixLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    juce::Slider roomSizeKnob;
    juce::Label roomSizeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> roomSizeAttachment;

    juce::Slider dampeningKnob;
    juce::Label dampeningLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampeningAttachment;

    juce::Slider stereoWidthKnob;
    juce::Label stereoWidthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stereoWidthAttachment;

    juce::Slider initialDelayKnob;
    juce::Label initialDelayLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> initialDelayAttachment;

    juce::Slider lowPassKnob;
    juce::Label lowPassLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowPassAttachment;

    juce::Slider highPassKnob;
    juce::Label highPassLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highPassAttachment;

    Spectrogram3DComponent spectrogram3D;

    CustomLookAndFeel customLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UmbraAudioProcessorEditor)
};
