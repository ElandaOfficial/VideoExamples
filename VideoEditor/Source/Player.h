/*
  ==============================================================================

    Copyright (c) 2019, Foleys Finest Audio - Daniel Walz
    All rights reserved.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
    OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
    OF THE POSSIBILITY OF SUCH DAMAGE.

  ==============================================================================

    Player.h
    Created: 31 Mar 2019 10:23:27pm
    Author:  Daniel Walz

  ==============================================================================
*/

#pragma once

#include <ff_meters/ff_meters.h>
#include <foleys_video_engine/foleys_video_engine.h>

//==============================================================================
/*
*/
class Player  : public juce::ChangeBroadcaster,
                public juce::ChangeListener
{
public:
    Player (juce::AudioDeviceManager& deviceManager, foleys::VideoEngine& engine, foleys::VideoPreview& preview);
    ~Player() override;

    void setClip (std::shared_ptr<foleys::AVClip> clip, bool needsPrepare);

    void start();
    void stop();
    bool isPlaying();

    void setPosition (double pts);
    void nextFrame();
    void previousFrame();

    double getCurrentTimeInSeconds() const;

    void setAuditionFile (const juce::File& file);
    void setAuditionSource (std::unique_ptr<juce::PositionableAudioSource> source, double sampleRate);
    void stopAudition();
    bool isAuditioning() const;

    foleys::LevelMeterSource& getMeterSource();

    void initialise();
    void shutDown();

    double getSampleRate() const;

    void changeListenerCallback (ChangeBroadcaster* sender) override;

    class MeasuredTransportSource : public juce::AudioTransportSource
    {
    public:
        MeasuredTransportSource() = default;

        void getNextAudioBlock (const juce::AudioSourceChannelInfo& info) override
        {
            AudioTransportSource::getNextAudioBlock (info);

            if (isPlaying())
            {
                juce::AudioBuffer<float> proxy (info.buffer->getArrayOfWritePointers(),
                                                info.buffer->getNumChannels(),
                                                info.startSample,
                                                info.numSamples);
                meterSource.measureBlock (proxy);
            }

            if (clipOutput)
            {
                for (int channel = 0; channel < info.buffer->getNumChannels(); ++channel)
                    juce::FloatVectorOperations::clip (info.buffer->getWritePointer (channel, info.startSample),
                                                       info.buffer->getReadPointer (channel, info.startSample),
                                                       -1.0f, 1.0f, info.numSamples);
            }
        }

        foleys::LevelMeterSource meterSource;

    private:
        const bool clipOutput = true;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeasuredTransportSource)
    };
private:
    juce::AudioDeviceManager& deviceManager;
    foleys::VideoEngine& videoEngine;

    juce::MixerAudioSource          mixingSource;
    std::shared_ptr<foleys::AVClip> clip;
    MeasuredTransportSource         transportSource;
    juce::AudioSourcePlayer         sourcePlayer;
    foleys::VideoPreview&           preview;

    std::unique_ptr<juce::PositionableAudioSource> auditionSource;
    juce::AudioTransportSource  auditionTransport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Player)
};
