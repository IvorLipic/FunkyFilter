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
    // Initialize the wavetable used for modulation
    initiateWavetable();

    // Create a ProcessSpec object to specify processing requirements
    juce::dsp::ProcessSpec spec;

    // Set the maximum block size, number of channels, and sample rate for processing (each channel is processed seperately)
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    // Prepare the left and right filters with the specified processing requirements
    filterLeft.prepare(spec);
    filterRight.prepare(spec);

    // Retrieve parameters from the parameter tree
    auto filterSettings = getFilterSettings(tree);

    // Update the filter with the current settings, sample rate, and block size
    updateFilter(filterSettings, sampleRate, samplesPerBlock);

    // Reset the phase for modulation
    phase = 0.f;
}

void FunkyFilterAudioProcessor::releaseResources()
{
    phase = 0.f;
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
    //JUCE generated
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Check if there is a play head to get the current position info
    if (auto* playHead = getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo info;

        // Get the current position info from the play head
        if (playHead->getCurrentPosition(info))
        {
            if (info.isPlaying)
            {
                // Update filter only when the transport is playing
                auto filterSettings = getFilterSettings(tree);
                updateFilter(filterSettings, getSampleRate(), buffer.getNumSamples());

                // Create an AudioBlock from the buffer for DSP processing
                juce::dsp::AudioBlock<float> block(buffer);
                auto leftBlock = block.getSingleChannelBlock(0);
                auto rightBlock = block.getSingleChannelBlock(1);

                // Create process contexts for left and right channels
                juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
                juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

                // Process the left and right channels through the filter
                filterLeft.process(leftContext);
                filterRight.process(rightContext);
            }
            else
            {
                // Reset phase when playback stops
                phase = 0.f;
            }
        }
    }
}

//==============================================================================
bool FunkyFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FunkyFilterAudioProcessor::createEditor()
{
    return new FunkyFilterAudioProcessorEditor (*this);
}

//==============================================================================
void FunkyFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    juce::MemoryOutputStream mos(destData, true);
    tree.state.writeToStream(mos);

}

void FunkyFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    auto valueTree = juce::ValueTree::readFromData(data, sizeInBytes);

    if (valueTree.isValid())
    {
        tree.replaceState(valueTree);
        updateFilter(getFilterSettings(tree), getSampleRate(), getBlockSize());
    }

}


// Retrieves values of parameters from the parameter tree and returns them as a FilterSettings structure.
// This function creates a wrapper around parameters for cleaner and more organized code.
FilterSettings getFilterSettings(juce::AudioProcessorValueTreeState& tree)
{
    FilterSettings settings;

    settings.lfoFreq = tree.getRawParameterValue("FilterFrequency")->load();
    settings.noteDurationIndex = tree.getRawParameterValue("NoteDuration")->load();
    settings.bpm = tree.getRawParameterValue("BPM")->load();
    settings.useNoteDuration = tree.getRawParameterValue("UseNoteDuration")->load() > 0.5f;
    settings.filterQuality = tree.getRawParameterValue("FilterQuality")->load();
    settings.minimumFrequency = tree.getRawParameterValue("MinimumFrequency")->load();
    settings.maximumFrequency = tree.getRawParameterValue("MaximumFrequency")->load();

    return settings;
}

//Creates coefficients for a band-pass filter using the specified filter frequency, filter quality (Q factor), and sample rate
Coefficients makeBandPassFilter(double filterFrequency, float filterQuality, double sampleRate)
{
    return juce::dsp::IIR::Coefficients<float>::makeBandPass(
        sampleRate,
        filterFrequency,
        filterQuality
    );
}

//Updates the existing filter coefficients with new coefficients
void updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    *old = *replacements;
}

void FunkyFilterAudioProcessor::updateFilter(const FilterSettings& filterSettings, double sampleRate, int blockSize)
{
    // Check if the filter frequency modulation should use note duration based on the parameter value
    if (filterSettings.useNoteDuration)
    {
        // Define an array of note durations (whole note, half note, etc.)
        float noteDurations[] = { 4.0f, 2.f, 1.f, 0.5f, 0.25f };

        // Get the current note duration from the array based on the parameter value
        float noteDuration = noteDurations[filterSettings.noteDurationIndex];

        // Calculate the modulation frequency based on BPM and note duration (LFO frequency)
        float modFrequency = filterSettings.bpm / (60 * noteDuration);

        // Calculate the phase increment for the LFO based on modulation frequency, sample rate, and block size
        increment = modFrequency * wavetableSize / (sampleRate / (float)blockSize);
    }
    else
    {
        // Calculate the phase increment for the LFO using a fixed frequency from the parameter tree
        increment = filterSettings.lfoFreq * wavetableSize / (sampleRate / (float)blockSize);
    }

    // Map the current phase value in the wavetable t   o a logarithmic frequency range
    currentFilterFrequency = juce::mapToLog10(wavetable[(int)phase], filterSettings.minimumFrequency, filterSettings.maximumFrequency);

    // Increment the phase and wrap it around using fmod to stay within the wavetable size
    phase = fmod(phase + increment, wavetableSize);

    // Generate new band-pass filter coefficients based on (fixed or calculated) frequency and quality factor
    auto filterCoefficients = makeBandPassFilter(currentFilterFrequency, filterSettings.filterQuality, sampleRate);

    // Update the left and right filter coefficients with the new ones
    updateCoefficients(filterLeft.coefficients, filterCoefficients);
    updateCoefficients(filterRight.coefficients, filterCoefficients);
}

//Parameters are created here
juce::AudioProcessorValueTreeState::ParameterLayout FunkyFilterAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
            "FilterFrequency",
            "FilterFrequency",
            juce::NormalisableRange<float>(0.1f, 20.0f, 0.1f, 0.5f),
            1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
            "FilterQuality",
            "FilterQuality",
            juce::NormalisableRange<float>(0.1f, 10.0f, 0.05f, 0.4f),
            1.0f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
            "MinimumFrequency",
            "MinimumFrequency",
            juce::NormalisableRange<float>(30.0f, 16000.0f, 1.0f, 0.25f),
            200.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
            "MaximumFrequency",
            "MaximumFrequency",
            juce::NormalisableRange<float>(30.0f, 16000.0f, 1.0f, 0.25f),
            5000.0f));
            
    layout.add(std::make_unique<juce::AudioParameterBool>(
            "UseNoteDuration",
            "UseNoteDuration",
            false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
            "BPM",
            "BPM",
            juce::NormalisableRange<float>(20.0f, 300.0f, 1.0f, 1.0f),
            120.0f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
            "NoteDuration",
            "NoteDuration",
            juce::StringArray{ "1 Note", "1/2 Note", "1/4 Note", "1/8 Note", "1/16 Note" },
            2));
    return layout;
}

//One cycle of a cosine wave
void FunkyFilterAudioProcessor::initiateWavetable()
{
    for (int i = 0; i < wavetableSize; i++) wavetable.insert(i, (cos(2.0 * juce::double_Pi * i / wavetableSize) + 1) / 2);
}


//Helper funnction to get the mod frequency to PluginEditor
double FunkyFilterAudioProcessor::getCurrentFilterFrequency() const
{
    return currentFilterFrequency;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FunkyFilterAudioProcessor();
}
