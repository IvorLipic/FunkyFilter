/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FunkyFilterAudioProcessor::FunkyFilterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

FunkyFilterAudioProcessor::~FunkyFilterAudioProcessor()
{
}

//==============================================================================
const juce::String FunkyFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FunkyFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FunkyFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FunkyFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FunkyFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FunkyFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FunkyFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FunkyFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FunkyFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void FunkyFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FunkyFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;

    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    filterLeft.prepare(spec);
    filterRight.prepare(spec);

    auto filterSettings = getFilterSettings(tree);

    auto filterCoefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, 
                                                                                filterSettings.filterFrequency, 
                                                                                filterSettings.filterQuality);
    *filterLeft.coefficients = *filterCoefficients;
    *filterRight.coefficients = *filterCoefficients;
}

void FunkyFilterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FunkyFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void FunkyFilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto filterSettings = getFilterSettings(tree);

    auto filterCoefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(getSampleRate(),
                                                                                filterSettings.filterFrequency,
                                                                                filterSettings.filterQuality);

    *filterLeft.coefficients = *filterCoefficients;
    *filterRight.coefficients = *filterCoefficients;

    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    filterLeft.process(leftContext);
    filterRight.process(rightContext);
}

//==============================================================================
bool FunkyFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FunkyFilterAudioProcessor::createEditor()
{
//    return new FunkyFilterAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void FunkyFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FunkyFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

FilterSettings getFilterSettings(juce::AudioProcessorValueTreeState& tree)
{
    FilterSettings settings;

    settings.filterFrequency = tree.getRawParameterValue("FilterFrequency")->load();
    settings.filterQuality = tree.getRawParameterValue("FilterQuality")->load();
    settings.minimumFrequency = tree.getRawParameterValue("MinimumFrequency")->load();
    settings.maximumFrequency = tree.getRawParameterValue("MaximumFrequency")->load();

    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout FunkyFilterAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
            "FilterFrequency",
            "FilterFrequency",
            juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
            1000.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
            "FilterQuality",
            "FilterQuality",
            juce::NormalisableRange<float>(0.1f, 10.0f, 0.05f, 1.0f),
            1.0f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
            "MinimumFrequency",
            "MinimumFrequency",
            juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 1.0f),
            200.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
            "MaximumFrequency",
            "MaximumFrequency",
            juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 1.0f),
            5000.0f));
    /*
    layout.add(std::make_unique<juce::AudioParameterFloat>(
            "ModFrequency",
            "ModFrequency",
            juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 1.0f),
            5000.0f));
    */
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FunkyFilterAudioProcessor();
}
