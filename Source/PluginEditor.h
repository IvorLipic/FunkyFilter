/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct MyRotarySlider : juce::Slider
{
    MyRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                    juce::Slider::TextEntryBoxPosition::NoTextBox)
    {

    }
};

//==============================================================================
/**
*/
class FunkyFilterAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FunkyFilterAudioProcessorEditor (FunkyFilterAudioProcessor&);
    ~FunkyFilterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FunkyFilterAudioProcessor& audioProcessor;

    MyRotarySlider filterFrequencySlider, filterQSlider, maximumFrequencySlider, minimumFrequencySlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FunkyFilterAudioProcessorEditor)
};
