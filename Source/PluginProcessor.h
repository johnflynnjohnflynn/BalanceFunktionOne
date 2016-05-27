/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "WDL/WDL/convoengine.h"    // Tale edition of WDL for threaded conv
#include "r8brain-free-src/CDSPResampler.h"


//==============================================================================
/**
*/
class BalanceFunktionOneAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    BalanceFunktionOneAudioProcessor();
    ~BalanceFunktionOneAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet) override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    double playbackSampleRate {0.0};
    double impulseSampleRate {0.0};

    ScopedPointer<AudioSampleBuffer> impulseJuceAudioSampleBufferL {nullptr};
    ScopedPointer<AudioSampleBuffer> impulseJuceAudioSampleBufferR {nullptr};

    WDL_ImpulseBuffer wdlImpulseL;              // these are stereo buffers!
    WDL_ImpulseBuffer wdlImpulseR;              // e.g. ins =     stereo L R
    WDL_ConvolutionEngine_Thread wdlEngineL;    //      out = 2 x stereo LL LR RL RR
    WDL_ConvolutionEngine_Thread wdlEngineR;

    static const int r8bBlockLength {64};

    ScopedPointer<r8b::CDSPResampler24> r8bResampler {nullptr};

    // Return resampling dest length
    inline int resampleLength(int sourceLen, double sourceRate, double destRate) const
    {
        return static_cast<int> (destRate / sourceRate * (double) sourceLen + 0.5);
    }

    template <class floatType>
    void resample (const floatType* src, int sourceLen, double sourceRate, floatType* dest, int dest_len, double destRate);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BalanceFunktionOneAudioProcessor)
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
