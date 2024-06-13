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

struct ResponseCurveComponent : juce::Component, juce::Timer
{
    ResponseCurveComponent(FunkyFilterAudioProcessor&);
    ~ResponseCurveComponent();

    void timerCallback() override;

    void paint(juce::Graphics& g) override;

    int pixelPositionForFrequency(double frequency, int width);

private:
    FunkyFilterAudioProcessor& audioProcessor;
    juce::dsp::IIR::Filter<float> filterRight, filterLeft;
};
//==============================================================================
/**
*/
class FunkyFilterAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    FunkyFilterAudioProcessorEditor(FunkyFilterAudioProcessor&);
    ~FunkyFilterAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    FunkyFilterAudioProcessor& audioProcessor;

    MyRotarySlider filterFrequencySlider, filterQSlider, maximumFrequencySlider, minimumFrequencySlider, bpmSlider;

    ResponseCurveComponent responseCurveComponent;

    juce::Label filterFrequencyLabel;
    juce::Label filterQLabel;
    juce::Label minimumFrequencyLabel;
    juce::Label maximumFrequencyLabel;
    juce::Label bpmLabel;
    juce::Label noteDurationLabel;

    juce::ToggleButton useNoteDurationButton;
    juce::ComboBox noteDurationComboBox;

    using apvts = juce::AudioProcessorValueTreeState;
    using sliderAttachment = apvts::SliderAttachment;

    sliderAttachment filterFrequencySliderAttachment, filterQSliderAttachment, maximumFrequencySliderAttachment, minimumFrequencySliderAttachment, bpmSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> useNoteDurationButtonAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> noteDurationComboBoxAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FunkyFilterAudioProcessorEditor)
};
