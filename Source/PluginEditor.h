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

struct ResponseCurveComponent : juce::Component, juce::AudioProcessorParameter::Listener, juce::Timer
{
    ResponseCurveComponent(FunkyFilterAudioProcessor&);
    ~ResponseCurveComponent();

    void parameterValueChanged(int parameterIndex, float newValue) override;

    virtual void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }

    void timerCallback() override;

    void paint(juce::Graphics& g) override;

    int pixelPositionForFrequency(double frequency, int width);

private:
    FunkyFilterAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged{ false };
    juce::dsp::IIR::Filter<float> filterRight, filterLeft;
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

    ResponseCurveComponent responseCurveComponent;

    using apvts = juce::AudioProcessorValueTreeState;
    using attachment = apvts::SliderAttachment;

    attachment filterFrequencySliderAttachment, filterQSliderAttachment, maximumFrequencySliderAttachment, minimumFrequencySliderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FunkyFilterAudioProcessorEditor)
};
 