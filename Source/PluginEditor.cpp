#include "PluginProcessor.h"
#include "PluginEditor.h"

ResponseCurveComponent::ResponseCurveComponent(FunkyFilterAudioProcessor& p) : audioProcessor(p)
{
    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
}

void ResponseCurveComponent::timerCallback()
{
    //Update coefficients
    auto filterSettings = getFilterSettings(audioProcessor.tree);
    auto filterCoefficients = makeBandPassFilter(audioProcessor.getCurrentFilterFrequency(), filterSettings.filterQuality, audioProcessor.getSampleRate());
    updateCoefficients(filter.coefficients, filterCoefficients);
    repaint();
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;

    g.fillAll(Colours::black);

    // Get the bounds of the response area
    auto filterResponseArea = getLocalBounds();

    // Get the width of the response area
    auto width = filterResponseArea.getWidth();

    // Get the sample rate from the audio processor
    auto sampleRate = audioProcessor.getSampleRate();

    // Prepare a vector to store magnitudes
    std::vector<double> magnitudes;
    magnitudes.resize(width);

    // Calculate the magnitude for each frequency
    for (int i = 0; i < width; i++)
    {
        // Map pixel position to a frequency (logarithmic scale)
        auto freq = mapToLog10(double(i) / double(width), 20.0, 20000.0);

        // Get the filter's magnitude response for the frequency
        double magnitude = filter.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        // Convert the magnitude to decibels and store in the vector
        magnitudes[i] = Decibels::gainToDecibels(magnitude);

    }
    // Create a path for the response curve
    Path responseCurve;

    // Get the vertical bounds of the response area
    const double outputMin = filterResponseArea.getBottom();
    const double outputMax = filterResponseArea.getY();

    // Lambda function to map decibel values to vertical position
    auto map = [outputMin, outputMax](double input)
        {
            // Map amplitude to window height (-12 dB min, 12 dB max)
            return jmap(input, -12.0, 12.0, outputMin, outputMax);
        };

    // Start the path at the first magnitude point
    responseCurve.startNewSubPath(filterResponseArea.getX(), map(magnitudes.front()));

    // Create the path for the response curve
    for (size_t i = 1; i < magnitudes.size(); ++i)
    {
        responseCurve.lineTo(filterResponseArea.getX() + i, map(magnitudes[i]));
    }

    // Draw the border of the response area
    g.setColour(Colours::white);
    g.drawRoundedRectangle(filterResponseArea.toFloat(), 5.f, 1.f);

    // Draw frequency scale
    g.setColour(Colours::yellow);
    for (int i = 30; i <= 20000; i *= 2)
    {
        auto xPos = pixelPositionForFrequency(i, width);
        g.setOpacity(0.3f);
        if (i < 15000) {
            g.drawVerticalLine(xPos, 0, getHeight());
            g.drawText(String(i) + " Hz", xPos - 25, getHeight() - 15, 50, 15, Justification::centred);
        }
        else {
            g.drawVerticalLine(xPos, 0, getHeight());
            g.drawText("15.36 k", xPos - 25, getHeight() - 15, 50, 15, Justification::centred);
        } 
    }

    // Draw dB scale
    g.setColour(Colours::blue);
    for (int i = -9; i <= 9; i += 3)
    {
        auto yPos = map(i);
        g.setOpacity(0.3f);
        g.drawHorizontalLine(yPos, 0, getWidth());   
        g.setOpacity(0.5f);
        g.drawText(String(i) + " dB", getWidth() - 40, yPos - 7, 35, 15, Justification::centredRight);
    }

    // Draw the amplitude response curve
    g.setColour(Colours::green);
    g.strokePath(responseCurve, PathStrokeType(2.f));

    // Get the filter settings from the audio processor
    auto filterSettings = getFilterSettings(audioProcessor.tree);

    // Draw vertical lines at the minimum and maximum frequencies
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
// Constructor for FunkyFilterAudioProcessorEditor
FunkyFilterAudioProcessorEditor::FunkyFilterAudioProcessorEditor(FunkyFilterAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), // Initialize base class and reference to audio processor
    responseCurveComponent(audioProcessor), // Initialize response curve component with the audio processor
    filterFrequencySliderAttachment(audioProcessor.tree, "FilterFrequency", filterFrequencySlider),
    filterQSliderAttachment(audioProcessor.tree, "FilterQuality", filterQSlider),
    maximumFrequencySliderAttachment(audioProcessor.tree, "MaximumFrequency", maximumFrequencySlider),
    minimumFrequencySliderAttachment(audioProcessor.tree, "MinimumFrequency", minimumFrequencySlider),
    bpmSliderAttachment(audioProcessor.tree, "BPM", bpmSlider),
    useNoteDurationButtonAttachment(audioProcessor.tree, "UseNoteDuration", useNoteDurationButton),
    noteDurationComboBoxAttachment(audioProcessor.tree, "NoteDuration", noteDurationComboBox)
{
    // Add components to the editor
    addAndMakeVisible(responseCurveComponent);
    addAndMakeVisible(filterFrequencySlider);
    addAndMakeVisible(filterQSlider);
    addAndMakeVisible(minimumFrequencySlider);
    addAndMakeVisible(maximumFrequencySlider);
    addAndMakeVisible(useNoteDurationButton);
    addAndMakeVisible(bpmSlider);
    addAndMakeVisible(noteDurationComboBox);

    // Set up and add labels
    filterFrequencyLabel.setText("Mod Frequency", juce::dontSendNotification);
    filterFrequencyLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterFrequencyLabel);

    filterQLabel.setText("Filter Q", juce::dontSendNotification);
    filterQLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterQLabel);

    minimumFrequencyLabel.setText("Minimum Frequency", juce::dontSendNotification);
    minimumFrequencyLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(minimumFrequencyLabel);

    maximumFrequencyLabel.setText("Maximum Frequency", juce::dontSendNotification);
    maximumFrequencyLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(maximumFrequencyLabel);

    bpmLabel.setText("BPM", juce::dontSendNotification);
    bpmLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bpmLabel);

    noteDurationLabel.setText("Note Duration", juce::dontSendNotification);
    noteDurationLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(noteDurationLabel);

    // Set up button text
    useNoteDurationButton.setButtonText("Use Note Duration (Click me)");
    
    // Populate note duration combo box
    noteDurationComboBox.addItemList({ "1 Note", "1/2 Note", "1/4 Note", "1/8 Note", "1/16 Note" }, 1);

    // Set bounds for labels
    filterFrequencyLabel.setBounds(75, 290, 150, 20); 
    bpmLabel.setBounds(75, 290, 150, 20);
    filterQLabel.setBounds(375, 290, 150, 20);
    minimumFrequencyLabel.setBounds(75, 380, 150, 20);
    maximumFrequencyLabel.setBounds(375, 380, 150, 20);

    // Set up slider value change listeners
    filterFrequencySlider.onValueChange = [this]() {
        filterFrequencyLabel.setText(juce::String(filterFrequencySlider.getValue(), 2), juce::dontSendNotification);
        };

    filterQSlider.onValueChange = [this]() {
        filterQLabel.setText(juce::String(filterQSlider.getValue(), 2), juce::dontSendNotification);
        };

    minimumFrequencySlider.onValueChange = [this]() {
        minimumFrequencyLabel.setText(juce::String(minimumFrequencySlider.getValue(), 2), juce::dontSendNotification);
        };

    maximumFrequencySlider.onValueChange = [this]() {
        maximumFrequencyLabel.setText(juce::String(maximumFrequencySlider.getValue(), 2), juce::dontSendNotification);
        };

    bpmSlider.onValueChange = [this]() {
        bpmLabel.setText(juce::String(bpmSlider.getValue(), 2), juce::dontSendNotification);
        };

    // Set up button click listener
    useNoteDurationButton.onClick = [this]() {
        bool useNoteDuration = useNoteDurationButton.getToggleState();
        filterFrequencySlider.setVisible(!useNoteDuration);
        filterFrequencyLabel.setVisible(!useNoteDuration);
        bpmSlider.setVisible(useNoteDuration);
        bpmLabel.setVisible(useNoteDuration);
        noteDurationComboBox.setVisible(useNoteDuration);
        };
    useNoteDurationButton.onClick();

    // Set the size of the window
    setSize(600, 400);
}

FunkyFilterAudioProcessorEditor::~FunkyFilterAudioProcessorEditor()
{
}

//==============================================================================
void FunkyFilterAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

//Bounds for components are set here
void FunkyFilterAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto filterResponseArea = bounds.removeFromTop(0.5 * bounds.getHeight());

    responseCurveComponent.setBounds(filterResponseArea);

    auto filterParametersArea = bounds.removeFromTop(0.5 * bounds.getHeight());
    auto filterFrequencySliderArea = filterParametersArea.removeFromLeft(0.5 * filterParametersArea.getWidth());
    filterFrequencySlider.setBounds(filterFrequencySliderArea);
    bpmSlider.setBounds(filterFrequencySliderArea);
    filterQSlider.setBounds(filterParametersArea);

    auto minimumFrequencyArea = bounds.removeFromLeft(0.5 * bounds.getWidth());
    minimumFrequencySlider.setBounds(minimumFrequencyArea);
    maximumFrequencySlider.setBounds(bounds);

    useNoteDurationButton.setBounds(filterFrequencySliderArea.getRight() - 80, filterFrequencySliderArea.getY() + 10, 150, 50);
    noteDurationComboBox.setBounds(filterFrequencySliderArea.getRight() - 80, filterFrequencySliderArea.getY() + 80, 150, 20);
}
