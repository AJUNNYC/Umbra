#pragma once

#include <JuceHeader.h>
#include "Reverb.h"
#include "FFTProcessor.h"

class UmbraAudioProcessor : public juce::AudioProcessor
{
public:
    UmbraAudioProcessor();
    ~UmbraAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // FFT related
    FFTProcessor fftProcessor;

    std::unique_ptr<Reverb> r;

    // Parameters
    juce::AudioProcessorValueTreeState parameters;

private:
    //float previousLowPassCutoff = -1.0f;
    //float previousHighPassCutoff = -1.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UmbraAudioProcessor)
};
