#include "PluginProcessor.h"
#include "PluginEditor.h"

UmbraAudioProcessor::UmbraAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr, juce::Identifier("UmbraParams"),
        {
            std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>("stereoWidth", "Stereo Width", 0.0f, 2.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>("lowPass", "Low Pass", 20.0f, 20000.0f, 20.0f),
            std::make_unique<juce::AudioParameterFloat>("highPass", "High Pass", 20.0f, 20000.0f, 20.0f),

            //  Add these missing parameters:
            std::make_unique<juce::AudioParameterFloat>("roomSize", "Room Size", 0.1f, 2.0f, 0.1f),
            std::make_unique<juce::AudioParameterFloat>("dampening", "Dampening", 20.0f, 20000.0f, 20.0f),
            std::make_unique<juce::AudioParameterFloat>("initialDelay", "Initial Delay", 0.0f, 0.1f, 0.0f)

        })

#endif
{
    // Your existing code (if any) here
}


UmbraAudioProcessor::~UmbraAudioProcessor()
{
}

const juce::String UmbraAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool UmbraAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool UmbraAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool UmbraAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double UmbraAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int UmbraAudioProcessor::getNumPrograms()
{
    return 1;
}

int UmbraAudioProcessor::getCurrentProgram()
{
    return 0;
}

void UmbraAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String UmbraAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void UmbraAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void UmbraAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    r = std::make_unique<Reverb>(sampleRate, samplesPerBlock);
}


void UmbraAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool UmbraAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    return true;
#endif
}
#endif

void UmbraAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    const int numChannels = getTotalNumInputChannels();
    const int numSamples = buffer.getNumSamples();

    for (int ch = numChannels; ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, numSamples);

    // === PARAMETERS ===
    float mix = *parameters.getRawParameterValue("mix");
    float stereoWidth = *parameters.getRawParameterValue("stereoWidth");
    float lowPass = *parameters.getRawParameterValue("lowPass");
    float highPass = *parameters.getRawParameterValue("highPass");
    float dampening = *parameters.getRawParameterValue("dampening");
    float roomSize = *parameters.getRawParameterValue("roomSize");
    float initialDelay = *parameters.getRawParameterValue("initialDelay");

    r->process(buffer, mix, stereoWidth, lowPass, highPass, dampening, roomSize, initialDelay);
    fftProcessor.pushSamples(buffer);
}



juce::AudioProcessorEditor* UmbraAudioProcessor::createEditor()
{
    return new UmbraAudioProcessorEditor(*this);
}

bool UmbraAudioProcessor::hasEditor() const
{
    return true;
}

void UmbraAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void UmbraAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}


// Thread-safe getters for editor

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new UmbraAudioProcessor();
}
