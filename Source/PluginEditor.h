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
public:
    //==============================================================================
    ResponseCurveComponent(FunkyFilterAudioProcessor&);
    ~ResponseCurveComponent();

    //==============================================================================
    void timerCallback() override;
    void paint(juce::Graphics& g) override;
    int pixelPositionForFrequency(double frequency, int width);

private:
    FunkyFilterAudioProcessor& audioProcessor;
    juce::dsp::IIR::Filter<float> filter;
};

//==============================================================================
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
    juce::ToggleButton useNoteDurationButton;
    juce::ComboBox noteDurationComboBox;
    juce::Label filterFrequencyLabel, filterQLabel, minimumFrequencyLabel, maximumFrequencyLabel, bpmLabel, noteDurationLabel;
    ResponseCurveComponent responseCurveComponent;
    
    using apvts = juce::AudioProcessorValueTreeState;
    using sliderAttachment = apvts::SliderAttachment;
    using buttonAttachment = apvts::ButtonAttachment;
    using comboBoxAttachment = apvts::ComboBoxAttachment;

    sliderAttachment filterFrequencySliderAttachment, filterQSliderAttachment, maximumFrequencySliderAttachment, minimumFrequencySliderAttachment, bpmSliderAttachment;
    buttonAttachment useNoteDurationButtonAttachment;
    comboBoxAttachment noteDurationComboBoxAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FunkyFilterAudioProcessorEditor)
};
