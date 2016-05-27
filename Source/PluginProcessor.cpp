/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
BalanceFunktionOneAudioProcessor::BalanceFunktionOneAudioProcessor()
{
    // Remember ...L&R are stereo files corresponding to true stereo output for left/right inputs

    // Put WAV impulses into Juce AudioFormatReaders
    WavAudioFormat wav;
    MemoryInputStream* misL {new MemoryInputStream {BinaryData::funktiononetsL_wav, BinaryData::funktiononetsL_wavSize, false}};
    MemoryInputStream* misR {new MemoryInputStream {BinaryData::funktiononetsR_wav, BinaryData::funktiononetsR_wavSize, false}};
    ScopedPointer<AudioFormatReader> audioReaderL {wav.createReaderFor (misL, true)};
    ScopedPointer<AudioFormatReader> audioReaderR {wav.createReaderFor (misR, true)};

    // Put AudioFormatReader into our IR AudioSampleBuffers                                         // 4 samples? interp?
    impulseJuceAudioSampleBufferL = new AudioSampleBuffer (audioReaderL->numChannels, audioReaderL->lengthInSamples + 4);
    impulseJuceAudioSampleBufferR = new AudioSampleBuffer (audioReaderR->numChannels, audioReaderR->lengthInSamples + 4);
    audioReaderL->read (impulseJuceAudioSampleBufferL, 0, audioReaderL->lengthInSamples + 4, 0, true, true);
    audioReaderR->read (impulseJuceAudioSampleBufferR, 0, audioReaderR->lengthInSamples + 4, 0, true, true);

    jassert (audioReaderL->sampleRate == audioReaderR->sampleRate); // TS L and TS R file should be same rate
    impulseSampleRate = audioReaderL->sampleRate;

    wdlEngineL.EnableThread(true);
    wdlEngineR.EnableThread(true);
}

BalanceFunktionOneAudioProcessor::~BalanceFunktionOneAudioProcessor()
{
}

//==============================================================================
const String BalanceFunktionOneAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BalanceFunktionOneAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BalanceFunktionOneAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double BalanceFunktionOneAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BalanceFunktionOneAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BalanceFunktionOneAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BalanceFunktionOneAudioProcessor::setCurrentProgram (int index)
{
}

const String BalanceFunktionOneAudioProcessor::getProgramName (int index)
{
    return String();
}

void BalanceFunktionOneAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void BalanceFunktionOneAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // First time and any time sample rate changes, re-initialise.
    if (playbackSampleRate != sampleRate)
    {
        playbackSampleRate = sampleRate;

        wdlImpulseL.SetNumChannels(2);  // doesn't seem to allow 4 channel
        wdlImpulseR.SetNumChannels(2);

        if (r8bResampler) delete r8bResampler;                                                              // is this safe?
        r8bResampler = new r8b::CDSPResampler24 {impulseSampleRate, playbackSampleRate, r8bBlockLength};

        // Resample the impulse responses.
        int lenL = resampleLength (impulseJuceAudioSampleBufferL->getNumSamples(), impulseSampleRate, playbackSampleRate);
        wdlImpulseL.SetLength (lenL);
        if (lenL)
        {
            resample (impulseJuceAudioSampleBufferL->getReadPointer(0),
                      impulseJuceAudioSampleBufferL->getNumSamples(),
                      impulseSampleRate,
                      wdlImpulseL.impulses[0].Get(),
                      lenL,
                      playbackSampleRate);
            resample (impulseJuceAudioSampleBufferL->getReadPointer(1),
                      impulseJuceAudioSampleBufferL->getNumSamples(),
                      impulseSampleRate,
                      wdlImpulseL.impulses[1].Get(),
                      lenL,
                      playbackSampleRate);
        }
        int lenR = resampleLength (impulseJuceAudioSampleBufferR->getNumSamples(), impulseSampleRate, playbackSampleRate);
        wdlImpulseR.SetLength (lenR);
        if (lenR)
        {
            resample (impulseJuceAudioSampleBufferR->getReadPointer(0),
                      impulseJuceAudioSampleBufferR->getNumSamples(),
                      impulseSampleRate,
                      wdlImpulseR.impulses[0].Get(),
                      lenR,
                      playbackSampleRate);
            resample (impulseJuceAudioSampleBufferR->getReadPointer(1),
                      impulseJuceAudioSampleBufferR->getNumSamples(),
                      impulseSampleRate,
                      wdlImpulseR.impulses[1].Get(),
                      lenR,
                      playbackSampleRate);
        }

        // Tie the impulse responses to the convolution engines.
        wdlEngineL.SetImpulse (&wdlImpulseL);
        wdlEngineR.SetImpulse (&wdlImpulseR);
    }

    //printf("Latency %i Samples\n", wdlEngineL.GetLatency());
}

void BalanceFunktionOneAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BalanceFunktionOneAudioProcessor::setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet)
{
    // Reject any bus arrangements that are not compatible with your plugin

    const int numChannels = preferredSet.size();

   #if JucePlugin_IsMidiEffect
    if (numChannels != 0)
        return false;
   #elif JucePlugin_IsSynth
    if (isInput || (numChannels != 1 && numChannels != 2))
        return false;
   #else
    if (numChannels != 1 && numChannels != 2)
        return false;

    if (! AudioProcessor::setPreferredBusArrangement (! isInput, bus, preferredSet))
        return false;
   #endif

    return AudioProcessor::setPreferredBusArrangement (isInput, bus, preferredSet);
}
#endif

void BalanceFunktionOneAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    /*for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }*/

    // Send input samples to the convolution engine.
    wdlEngineL.Add (buffer.getArrayOfWritePointers(), buffer.getNumSamples(), 2);
    wdlEngineR.Add (buffer.getArrayOfWritePointers(), buffer.getNumSamples(), 2);

    float *outL = buffer.getWritePointer(0);
    float *outR = buffer.getWritePointer(1);

    // find number available samples for the engine
    int numAvailL = std::min (wdlEngineL.Avail(buffer.getNumSamples()), buffer.getNumSamples());
    int numAvailR = std::min (wdlEngineR.Avail(buffer.getNumSamples()), buffer.getNumSamples());

    // Output samples from the convolution engine.
    if (numAvailL > 0)
    {
        // Apply the true stereo convolution (in L R -> out LL LR RL RR)
        WDL_FFT_REAL* convoLL = wdlEngineL.Get()[0];
        WDL_FFT_REAL* convoLR = wdlEngineL.Get()[1];
        WDL_FFT_REAL* convoRL = wdlEngineR.Get()[0];
        WDL_FFT_REAL* convoRR = wdlEngineR.Get()[1];
        for (int i = 0; i < numAvailL; ++i)
        {
            *outL++ = *convoLL++ + *convoRL++;        // convolved only, no dry
            *outR++ = *convoLR++ + *convoRR++;
        }

        // Remove the sample block from the convolution engine's buffer.
        wdlEngineL.Advance(numAvailL);
        wdlEngineR.Advance(numAvailR);
    }
}

//==============================================================================
bool BalanceFunktionOneAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* BalanceFunktionOneAudioProcessor::createEditor()
{
    return new BalanceFunktionOneAudioProcessorEditor (*this);
}

//==============================================================================
void BalanceFunktionOneAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BalanceFunktionOneAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BalanceFunktionOneAudioProcessor();
}

//==============================================================================
template <class floatType>
void BalanceFunktionOneAudioProcessor::resample(const floatType* source,
                                             int sourceLen,
                                             double sourceRate,
                                             floatType* dest,
                                             int destLen,
                                             double destRate)
{
    if (destLen == sourceLen)
    {
        // Copy
        for (int i = 0; i < destLen; ++i)
            *dest++ = static_cast<floatType> (*source++);
        return;
    }

    // Resample using r8brain-free-src.
    double scale = sourceRate / destRate;
    while (destLen > 0)
    {
        double buf[r8bBlockLength];
        double* p = buf;
        int n = r8bBlockLength;

        if (n > sourceLen)
            n = sourceLen;

        for (int i = 0; i < n; ++i)
            *p++ = static_cast<double> (*source++);               // copy source to buf
        if (n < r8bBlockLength)
            memset (p, 0, (r8bBlockLength - n) * sizeof(double)); // fill with zeroes
        sourceLen -= n;
        
        n = r8bResampler->process (buf, r8bBlockLength, p);       // process() returns
        if (n > destLen)                                          // size of output buffer
            n = destLen;
        for (int i = 0; i < n; ++i)
            *dest++ = static_cast<floatType> (scale * *p++);      // scale back to unity
        destLen -= n;
    }
    
    r8bResampler->clear();
}
