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

    Library.h
    Created: 30 Mar 2019 4:45:01pm
    Author:  Daniel Walz

  ==============================================================================
*/

#pragma once

#include <foleys_video_engine/foleys_video_engine.h>

class Player;

//==============================================================================
/*
*/
class Library    : public juce::Component
{
public:
    Library (Player& player, foleys::VideoEngine& engine);

    void paint (juce::Graphics&) override;
    void resized() override;

    class MediaList  : public Component,
                       private juce::FileBrowserListener
    {
    public:
        MediaList (Player& player, juce::TimeSliceThread& readThread, const juce::File& root, std::unique_ptr<juce::FileFilter> filter);
        ~MediaList() override;

        void resized() override;

        void selectionChanged() override {}
        void fileClicked (const juce::File &file, const juce::MouseEvent &e) override;
        void fileDoubleClicked (const juce::File &file) override;
        void browserRootChanged (const juce::File &) override {}

    private:
        Player&                           player;
        std::unique_ptr<juce::FileFilter> filter;
        juce::DirectoryContentsList       contents;
        juce::FileTreeComponent           fileTree  { contents };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MediaList)
    };
private:
    juce::TimeSliceThread directoryThread { "Directory read thread" };
    
    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };

    foleys::VideoEngine& videoEngine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Library)
};
