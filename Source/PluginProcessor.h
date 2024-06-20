#pragma once

#include <JuceHeader.h>

//Data structure for parameters
struct FilterSettings
{
    float filterQuality{ 1.f }, minimumFrequency{ 0 }, maximumFrequency{ 0 }, bpm{ 120 }, lfoFreq{ 1 };
    bool useNoteDuration{ false };
    int noteDurationIndex{ 0 };
};
using Coefficients = juce::dsp::IIR::Filter<float>::CoefficientsPtr;

//=============================GLOBAL METHODS==================================

// Retrieves values of parameters from the parameter tree and returns them as a FilterSettings structure.
// This function creates a wrapper around parameters for cleaner and more organized code.
inline FilterSettings getFilterSettings(juce::AudioProcessorValueTreeState& tree)
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
inline Coefficients makeBandPassFilter(double filterFrequency, float filterQuality, double sampleRate)
{
    return juce::dsp::IIR::Coefficients<float>::makeBandPass(
        sampleRate,
        filterFrequency,
        filterQuality
    );
}

//Updates the existing filter coefficients with new coefficients
inline void updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    *old = *replacements;
}

//==============================================================================
class FunkyFilterAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    FunkyFilterAudioProcessor();
    ~FunkyFilterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

public:
    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState tree {*this, nullptr, "Parameters", createParameterLayout()};

    //==============================================================================
    void initiateWavetable();
    double getCurrentFilterFrequency() const;

private:
    //==============================================================================
    juce::dsp::IIR::Filter<float> filterRight, filterLeft;
    juce::Array<float> wavetable;
    double wavetableSize = 1024;
    double phase = 0;
    double increment;
    double currentFilterFrequency = 1000.0;

    //==============================================================================
    void updateFilter(const FilterSettings& filterSettings, double sampleRate, int blockSize);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FunkyFilterAudioProcessor)
};
