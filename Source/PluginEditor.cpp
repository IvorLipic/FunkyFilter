/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FunkyFilterAudioProcessorEditor::FunkyFilterAudioProcessorEditor (FunkyFilterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    filterFrequencySliderAttachment(audioProcessor.tree, "FilterFrequency", filterFrequencySlider),
    filterQSliderAttachment(audioProcessor.tree, "FilterQuality", filterQSlider),
    maximumFrequencySliderAttachment(audioProcessor.tree, "MaximumFrequency", maximumFrequencySlider),
    minimumFrequencySliderAttachment(audioProcessor.tree, "MinimumFrequency", minimumFrequencySlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    addAndMakeVisible(filterFrequencySlider);
    addAndMakeVisible(filterQSlider);
    addAndMakeVisible(minimumFrequencySlider);
    addAndMakeVisible(maximumFrequencySlider);

    const auto& params = audioProcessor.getParameters();
    for (auto param : params) param->addListener(this);//Adding listeners to all parameters

    startTimerHz(60);

    setSize (600, 400);
}

FunkyFilterAudioProcessorEditor::~FunkyFilterAudioProcessorEditor()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) param->removeListener(this);
}

//==============================================================================
void FunkyFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);

    auto bounds = getLocalBounds();
    auto filterResponseArea = bounds.removeFromTop(0.5 * bounds.getHeight());

    auto width = filterResponseArea.getWidth();
    
    auto sampleRate = audioProcessor.getSampleRate();

    std::vector<double> magnitudes;
    magnitudes.resize(width);

    for (int i = 0; i < width; i++)
    {
        auto freq = mapToLog10(double(i) / double(width), 20.0, 20000.0); //Mapiranje pixela na frekvenciju

        double magnitude = filterLeft.coefficients->getMagnitudeForFrequency(freq, sampleRate);//Amplitudni odziv filtera za neku frekvenciju

        magnitudes[i] = Decibels::gainToDecibels(magnitude);//Spremanje amplitude u decibelima u listu

    }
    Path responseCurve;

    const double outputMin = filterResponseArea.getBottom();//Minimalna vertikalna pozicija prozora
    const double outputMax = filterResponseArea.getY();//Maksimalna vertikalna pozicija prozora
    auto map = [outputMin, outputMax](double input)
        {
            /*
            * Mapiranje amplitude na visinu prozora (-6 Db min, 6 Db max)
            */
            return jmap(input, -12.0, 12.0, outputMin, outputMax);
        };

    responseCurve.startNewSubPath(filterResponseArea.getX(), map(magnitudes.front()));

    for (size_t i = 1; i < magnitudes.size(); ++i)
    {
        responseCurve.lineTo(filterResponseArea.getX() + i, map(magnitudes[i]));
    }


    //Iscrtavanje prozora
    g.setColour(Colours::white);
    g.drawRoundedRectangle(filterResponseArea.toFloat(), 5.f, 1.f);


    //Iscrtavanje amplitudnog odziva (krivulje)
    g.setColour(Colours::green);
    g.strokePath(responseCurve, PathStrokeType(2.f));
    
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

void FunkyFilterAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void FunkyFilterAudioProcessorEditor::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
        //Update coefficients
        auto filterSettings = getFilterSettings(audioProcessor.tree);
        auto filterCoefficients = makeBandPassFilter(filterSettings, audioProcessor.getSampleRate());
        updateCoefficients(filterLeft.coefficients, filterCoefficients);
        updateCoefficients(filterRight.coefficients, filterCoefficients);

        repaint();
    }
}
