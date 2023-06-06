/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ProjectAAudioProcessor::ProjectAAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

ProjectAAudioProcessor::~ProjectAAudioProcessor()
{
}

//==============================================================================
const juce::String ProjectAAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ProjectAAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ProjectAAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ProjectAAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ProjectAAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ProjectAAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ProjectAAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ProjectAAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ProjectAAudioProcessor::getProgramName (int index)
{
    return {};
}

void ProjectAAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ProjectAAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    leftChannel.prepare(spec);
    rightChannel.prepare(spec);
    
//    auto chainSettings = getChainSettings(apvts);
//
//    auto lowCutCoeff = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.low_cut,
//                                                                                                   sampleRate,
//                                                                                                   2 * (chainSettings.lowCutSlope + 1));
//    auto& leftLowCut = leftChannel.get<ChainPositions::LowCut>();
//
//    leftLowCut.setBypassed<0>(true);
//    leftLowCut.setBypassed<1>(true);
//    leftLowCut.setBypassed<2>(true);
//    leftLowCut.setBypassed<3>(true);
//
//    *leftLowCut.get<0>().coefficients = *lowCutCoeff[0];
//    leftLowCut.setBypassed<0>(false);
//
//    auto& rightLowCut = rightChannel.get<ChainPositions::LowCut>();
//
//    rightLowCut.setBypassed<0>(true);
//    rightLowCut.setBypassed<1>(true);
//    rightLowCut.setBypassed<2>(true);
//    rightLowCut.setBypassed<3>(true);
//
//    *rightLowCut.get<0>().coefficients = *lowCutCoeff[0];
//    rightLowCut.setBypassed<0>(false);
}

void ProjectAAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ProjectAAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ProjectAAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    auto chainSettings = getChainSettings(apvts);
    
    
    auto lowCutCoeff = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.low_cut,
                                                                                                   getSampleRate(),
                                                                                                   2 * (chainSettings.lowCutSlope + 1));
    auto& leftLowCut = leftChannel.get<ChainPositions::LowCut>();

    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);

    *leftLowCut.get<0>().coefficients = *lowCutCoeff[0];
    leftLowCut.setBypassed<0>(false);
    
    auto& rightLowCut = rightChannel.get<ChainPositions::LowCut>();
    
    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);

    *rightLowCut.get<0>().coefficients = *lowCutCoeff[0];
    rightLowCut.setBypassed<0>(false);
    
    float gain = *apvts.getRawParameterValue("fuzz") * 60;

    float volume = *apvts.getRawParameterValue("volume");
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getReadPointer(channel);

            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                float drySample = channelData[sample];
                float wetSignal = drySample * juce::Decibels::decibelsToGain(gain);
                if (wetSignal > 0.99) {
                    wetSignal *= 0.99 / std::abs(wetSignal);
                }

                buffer.getWritePointer(channel)[sample] = wetSignal;
                buffer.getWritePointer(channel)[sample] *= volume;
            }
        }
    
    juce::dsp::AudioBlock<float> block(buffer);
    
  
    
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    
    leftChannel.process(leftContext);
    rightChannel.process(rightContext);
    
    

    
    
}

//==============================================================================
bool ProjectAAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ProjectAAudioProcessor::createEditor()
{
//    return new ProjectAAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void ProjectAAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ProjectAAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ProjectAAudioProcessor();
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts) {
    ChainSettings settings;
    
    settings.volume = apvts.getRawParameterValue("volume")->load();
    settings.fuzz = apvts.getRawParameterValue("fuzz")->load();
    settings.low_cut = apvts.getRawParameterValue("low cut")->load();
    settings.high_cut = apvts.getRawParameterValue("high cut")->load();
//    settings.lowCutSlope = apvts.getRawParameterValue("LowCut Slope")->load();
//    settings.highCutSlope = apvts.getRawParameterValue("HighCut Slope")->load();
    
    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout ProjectAAudioProcessor::CreateParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("volume",
                                                           "volume",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.0f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("fuzz",
                                                           "fuzz",
                                                           juce::NormalisableRange<float>(0.f, 100.f, 1.f, 1.f),
                                                           0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("low cut",
                                                           "low cut",
                                                           juce::NormalisableRange<float>(20.f, 5000.f, 1.f, 1.f),
                                                           20.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("high cut",
                                                           "high cut",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
                                                           20000.f));
//    juce::StringArray stringArray;
//    for (int i = 0; i < 4; i++) {
//        juce::String str;
//        str << (12 * i * 12);
//        str << " db/oct";
//        stringArray.add(str);
//    }
//
//    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));
//    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));
    return layout;
}
