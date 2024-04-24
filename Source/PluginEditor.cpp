/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ResponseCurveComponent::ResponseCurveComponent(FunkyFilterAudioProcessor& p) : audioProcessor(p)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) param->addListener(this);//Adding listeners to all parameters

    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) param->removeListener(this);
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void ResponseCurveComponent::timerCallback()
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

void ResponseCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;

    g.fillAll(Colours::black);

    auto filterResponseArea = getLocalBounds();

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
            * Mapiranje amplitude na visinu prozora (-12 Db min, 12 Db max)
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

    auto filterSettings = getFilterSettings(audioProcessor.tree);

    //Iscrtavanje vertikalnih linija na min i max freq
    g.setColour(juce::Colours::red);
    int xMinPos = pixelPositionForFrequency(filterSettings.minimumFrequency, width);
    int xMaxPos = pixelPositionForFrequency(filterSettings.maximumFrequency, width);
    g.drawLine(xMinPos, 0, xMinPos, getHeight(), 2.0f);
    g.drawLine(xMaxPos, 0, xMaxPos, getHeight(), 2.0f);

}
int ResponseCurveComponent::pixelPositionForFrequency(double frequency, int width)
{
    // Calculate the logarithmic range
    double logMin = std::log10(20.0);
    double logMax = std::log10(20000.0);
    double logRange = logMax - logMin;

    // Calculate the logarithmic value for the given frequency
    double logFreq = std::log10(frequency);

    // Map the logarithmic value to the range [0, 1]
    double normValue = (logFreq - logMin) / logRange;

    // Map the normalized value to the pixel position
    return static_cast<int>(normValue * width);
}
//==============================================================================
FunkyFilterAudioProcessorEditor::FunkyFilterAudioProcessorEditor (FunkyFilterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    responseCurveComponent(audioProcessor),
    filterFrequencySliderAttachment(audioProcessor.tree, "FilterFrequency", filterFrequencySlider),
    filterQSliderAttachment(audioProcessor.tree, "FilterQuality", filterQSlider),
    maximumFrequencySliderAttachment(audioProcessor.tree, "MaximumFrequency", maximumFrequencySlider),
    minimumFrequencySliderAttachment(audioProcessor.tree, "MinimumFrequency", minimumFrequencySlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    addAndMakeVisible(responseCurveComponent);
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
    g.fillAll(juce::Colours::black);
}

void FunkyFilterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    auto filterResponseArea = bounds.removeFromTop(0.5 * bounds.getHeight());

    responseCurveComponent.setBounds(filterResponseArea);

    auto filterParametersArea = bounds.removeFromTop(0.5 * bounds.getHeight());
    auto filterFrequencySliderArea = filterParametersArea.removeFromLeft(0.5 * filterParametersArea.getWidth());
    filterFrequencySlider.setBounds(filterFrequencySliderArea);
    filterQSlider.setBounds(filterParametersArea);

    auto minimumFrequencyArea = bounds.removeFromLeft(0.5 * bounds.getWidth());
    minimumFrequencySlider.setBounds(minimumFrequencyArea);
    maximumFrequencySlider.setBounds(bounds);
}
