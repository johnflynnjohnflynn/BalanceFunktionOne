/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class BalanceFunktionOneAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    BalanceFunktionOneAudioProcessorEditor (BalanceFunktionOneAudioProcessor&);
    ~BalanceFunktionOneAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    Image img;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BalanceFunktionOneAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BalanceFunktionOneAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
