/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FunkyFilterAudioProcessorEditor::FunkyFilterAudioProcessorEditor (FunkyFilterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    addAndMakeVisible(filterFrequencySlider);
    addAndMakeVisible(filterQSlider);
    addAndMakeVisible(minimumFrequencySlider);
    addAndMakeVisible(maximumFrequencySlider);

    setSize (600, 400);
}

FunkyFilterAudioProcessorEditor::~FunkyFilterAudioProcessorEditor()
{
}

//==============================================================================
void FunkyFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void FunkyFilterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    auto filterResponseArea = bounds.removeFromTop(0.5 * bounds.getHeight());

    auto filterParametersArea = bounds.removeFromTop(0.5 * bounds.getHeight());
    auto filterFrequencySliderArea = filterParametersArea.removeFromLeft(0.5 * filterParametersArea.getWidth());
    filterFrequencySlider.setBounds(filterFrequencySliderArea);
    filterQSlider.setBounds(filterParametersArea);

    auto minimumFrequencyArea = bounds.removeFromLeft(0.5 * bounds.getWidth());
    minimumFrequencySlider.setBounds(minimumFrequencyArea);
    maximumFrequencySlider.setBounds(bounds);
}