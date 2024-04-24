/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct FilterSettings
{
    float filterFrequency{ 0 }, filterQuality{ 1.f }, minimumFrequency{ 0 }, maximumFrequency{ 0 };
};

FilterSettings getFilterSettings(juce::AudioProcessorValueTreeState& apvts);


using Coefficients = juce::dsp::IIR::Filter<float>::CoefficientsPtr;

void updateCoefficients(Coefficients& old, const Coefficients& replacements);

Coefficients makeBandPassFilter(const FilterSettings& filterSettings, double sampleRate);
//==============================================================================
/**
*/
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

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState tree {*this, nullptr, "Parameters", createParameterLayout()};

private:
    juce::dsp::IIR::Filter<float> filterRight, filterLeft;

    void updateFilter(const FilterSettings& filterSettings);
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FunkyFilterAudioProcessor)
};
