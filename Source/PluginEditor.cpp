
/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
BalanceFunktionOneAudioProcessorEditor::BalanceFunktionOneAudioProcessorEditor (BalanceFunktionOneAudioProcessor& p)
    : AudioProcessorEditor (&p),
      img {ImageCache::getFromMemory (BinaryData::funktiononeirs_jpg,
                                      BinaryData::funktiononeirs_jpgSize)},
      processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 400);
}

BalanceFunktionOneAudioProcessorEditor::~BalanceFunktionOneAudioProcessorEditor()
{
}

//==============================================================================
void BalanceFunktionOneAudioProcessorEditor::paint (Graphics& g)
{
    g.drawImage (img, 0, 0, 400, 400, 0, 0, 645, 645);
}

void BalanceFunktionOneAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
