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

    TransportControl.h
    Created: 1 Apr 2019 11:54:16pm
    Author:  Daniel Walz

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/*
*/
class TransportControl    : public juce::Component,
                            private juce::ChangeListener,
                            private juce::Timer
{
public:
    explicit TransportControl (Player& player);
    ~TransportControl() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

    void changeListenerCallback (juce::ChangeBroadcaster* sender) override;

private:
    Player& player;
    
    juce::TextButton zero { NEEDS_TRANS ("Return") };
    juce::TextButton stop { NEEDS_TRANS ("Stop") };
    juce::TextButton play { NEEDS_TRANS ("Play") };
    juce::TextButton settings { NEEDS_TRANS ("Settings") };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransportControl)
};
