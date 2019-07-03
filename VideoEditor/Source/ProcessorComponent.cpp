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

    ProcessorComponent.cpp
    Created: 25 May 2019 11:18:43am
    Author:  Daniel Walz

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "ProcessorComponent.h"
#include "Player.h"

//==============================================================================

namespace IDs
{
    static Identifier collapsed { "collapsed" };
}

ProcessorComponent::ProcessorComponent (foleys::ProcessorController& controllerToUse, Player& player)
  : controller (controllerToUse)
{
    active.setClickingTogglesState (true);
    active.setToggleState (controller.isActive(), dontSendNotification);
    addAndMakeVisible (active);
    active.onStateChange = [&]
    {
        controller.setActive (active.getToggleState());
    };
    active.setColour (TextButton::buttonOnColourId, Colours::green);

    addChildComponent (editor);
    if (auto* processor = controller.getAudioProcessor())
    {
        editor.setVisible (processor->hasEditor());
        editor.onClick = [&]
        {
            if (auto* processor = controller.getAudioProcessor())
                showProcessorEditor (processor->createEditor(), processor->getName() + " - " + controller.getOwningClipDescriptor().getDescription());

            sendChangeMessage();
        };
    }

    collapse.setClickingTogglesState (true);
    collapse.setToggleState (isCollapsed(), dontSendNotification);
    addAndMakeVisible (collapse);
    collapse.onClick = [&]
    {
        controller.getProcessorState().setProperty (IDs::collapsed, collapse.getToggleState(), nullptr);
        sendChangeMessage();
    };

    addAndMakeVisible (remove);
    remove.onClick = [&]
    {
        controller.getOwningClipDescriptor().removeProcessor (&controller);
    };

    for (auto& parameter : controller.getParameters())
    {
        auto component = std::make_unique<ParameterComponent>(controller.getOwningClipDescriptor(), *parameter, player);
        addAndMakeVisible (component.get());
        parameterComponents.push_back (std::move (component));
    }

    editor.setConnectedEdges (Button::ConnectedOnRight);
    remove.setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight);
    collapse.setConnectedEdges (Button::ConnectedOnLeft);

    controller.getOwningClipDescriptor().getOwningClip().addTimecodeListener (this);
    controller.getOwningClipDescriptor().addListener (this);
}

ProcessorComponent::~ProcessorComponent()
{
    controller.getOwningClipDescriptor().removeListener (this);
    controller.getOwningClipDescriptor().getOwningClip().removeTimecodeListener (this);
}

void ProcessorComponent::paint (Graphics& g)
{
    g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0);

    g.setColour (Colours::grey);
    g.drawRoundedRectangle (getLocalBounds().toFloat(), 6.0, 1);

    g.setColour (Colours::silver);
    g.setFont (16.0f);

    auto area = getLocalBounds();

    g.drawText (controller.getName(), area.removeFromTop (24).reduced (33, 3),
                Justification::left, true);
}

void ProcessorComponent::resized()
{
    auto area = getLocalBounds().reduced (3);
    auto heading = area.removeFromTop (24).reduced (3, 0);
    active.setBounds (heading.removeFromLeft (24));
    collapse.setBounds (heading.removeFromRight (24));
    remove.setBounds (heading.removeFromRight (24));
    editor.setBounds (heading.removeFromRight (24));

    auto collapsed = isCollapsed();

    for (auto& c : parameterComponents)
    {
        c->setVisible (! collapsed);
        if (! collapsed)
            c->setBounds (area.removeFromTop (40));
    }
}

void ProcessorComponent::mouseDrag (const MouseEvent& event)
{
    if (event.getDistanceFromDragStart() > 5)
    {
        if (auto* dndContainer = findParentComponentOfClass<DragAndDropContainer>())
            dndContainer->startDragging (controller.getProcessorState().toXmlString(), this);
    }
}

void ProcessorComponent::showProcessorEditor (AudioProcessorEditor* editor, const String& title)
{
    audioProcessorWindow = std::make_unique<AudioProcessorWindow>(editor, title);
    audioProcessorWindow->centreAroundComponent (getTopLevelComponent(), audioProcessorWindow->getWidth(), audioProcessorWindow->getHeight());
}

bool ProcessorComponent::isCollapsed() const
{
    return controller.getProcessorState().getProperty (IDs::collapsed, false);
}

int ProcessorComponent::getHeightForWidth(int width) const
{
    // todo: adapt to width
    ignoreUnused (width);

    if (isCollapsed())
        return 40;

    return int (40 + 40 * controller.getNumParameters());
}

void ProcessorComponent::timecodeChanged (int64_t count, double seconds)
{
    auto localTime = controller.getOwningClipDescriptor().getClipTimeInDescriptorTime (seconds);
    for (auto& c : parameterComponents)
        c->updateForTime (localTime);
}

void ProcessorComponent::processorControllerToBeDeleted (const foleys::ProcessorController* controllerToBeDeleted)
{
    if (controllerToBeDeleted == &controller)
        audioProcessorWindow.reset();
}

void ProcessorComponent::parameterAutomationChanged (const foleys::ParameterAutomation*)
{
    auto seconds = controller.getOwningClipDescriptor().getCurrentPTS();
    for (auto& c : parameterComponents)
        c->updateForTime (seconds);
}

const foleys::ProcessorController* ProcessorComponent::getProcessorController() const
{
    return &controller;
}

//==============================================================================

class ParameterSlider : public ProcessorComponent::ParameterComponent::ParameterWidget
{
public:
    ParameterSlider (foleys::ParameterAutomation& parameter, foleys::ClipDescriptor& clip)
    {
        const auto numSteps = parameter.getNumSteps();

        NormalisableRange<double> range;
        if (numSteps > 0)
            range.interval = 1.0 / numSteps;

        valueSlider.onDragStart = [&]
        {
            parameter.startAutomationGesture();
            dragging = true;
        };

        valueSlider.onDragEnd = [&]
        {
            dragging = false;
            parameter.finishAutomationGesture();
        };

        valueSlider.onValueChange = [&]
        {
            if (dragging)
            {
                parameter.setValue (clip.getCurrentPTS(), valueSlider.getValue());
                if (parameter.isVideoParameter())
                    clip.getOwningClip().invalidateVideo();
            }
        };

        valueSlider.textFromValueFunction = [&parameter](double value) { return parameter.getText (value); };
        valueSlider.valueFromTextFunction = [&parameter](String text) { return parameter.getValueForText (text); };

        valueSlider.setNormalisableRange (range);
    }

    void setValue (double value) override
    {
        if (!dragging)
            valueSlider.setValue (value);
    }

    double getValue() const override
    {
        return valueSlider.getValue();
    }

    juce::Component& getComponent() override
    {
        return valueSlider;
    }

private:
    Slider valueSlider { Slider::LinearHorizontal, Slider::TextBoxRight };
    bool dragging = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterSlider)
};

class ParameterChoice : public ProcessorComponent::ParameterComponent::ParameterWidget
{
public:
    ParameterChoice (foleys::ParameterAutomation& parameter, foleys::ClipDescriptor& clip)
    {
        choice.addItemList (parameter.getAllValueStrings(), 1);
        choice.onChange = [&]
        {
            const auto numChoices = choice.getNumItems();
            parameter.setValue (choice.getSelectedItemIndex() / (numChoices - 1.0));
        };
    }

    void setValue (double value) override
    {
        auto numChoices = choice.getNumItems();
        if (numChoices > 1)
            choice.setSelectedItemIndex (value * (numChoices - 1.0));
        else
            choice.setSelectedItemIndex (0);
    }

    double getValue() const override
    {
        auto numChoices = choice.getNumItems();
        if (numChoices > 1)
            return roundToInt (choice.getSelectedItemIndex() / (numChoices - 1.0));

        return 0;
    }

    juce::Component& getComponent() override
    {
        return choice;
    }

private:
    ComboBox choice;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterChoice)
};

class ParameterSwitch : public ProcessorComponent::ParameterComponent::ParameterWidget
{
public:
    ParameterSwitch (foleys::ParameterAutomation& parameter, foleys::ClipDescriptor& clip)
    {
        button.setButtonText (parameter.getText (1.0f));
        button.setClickingTogglesState (true);
        button.onClick = [&]
        {
            parameter.startAutomationGesture();
            parameter.setValue (clip.getCurrentPTS(), button.getToggleState() ? 1.0 : 0.0);
            parameter.finishAutomationGesture();
        };
    }

    void setValue (double value) override
    {
        button.setToggleState (value > 0.5, dontSendNotification);
    }

    double getValue() const override
    {
        return button.getToggleState() ? 1.0 : 0.0;
    }

    juce::Component& getComponent() override
    {
        return button;
    }

private:
    TextButton button;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterSwitch)
};

//==============================================================================

ProcessorComponent::ParameterComponent::ParameterComponent (foleys::ClipDescriptor& clipToControl,
                                                            foleys::ParameterAutomation& parameterToControl,
                                                            Player& player)
  : clip (clipToControl),
    parameter (parameterToControl)
{
    prev.setConnectedEdges (TextButton::ConnectedOnRight);
    next.setConnectedEdges (TextButton::ConnectedOnRight | TextButton::ConnectedOnLeft);
    add.setConnectedEdges (TextButton::ConnectedOnLeft);

    addAndMakeVisible (prev);
    addAndMakeVisible (next);
    addAndMakeVisible (add);

    auto numSteps = parameter.getNumSteps();

    auto options = parameter.getAllValueStrings();
    if (numSteps == 2)
    {
        widget = std::make_unique<ParameterSwitch>(parameter, clip);
    }
    else if (! options.isEmpty())
    {
        widget = std::make_unique<ParameterChoice>(parameter, clip);
    }
    else
    {
        widget = std::make_unique<ParameterSlider>(parameter, clip);
    }

    widget->setValue (parameter.getValue());
    addAndMakeVisible (widget->getComponent());

    add.onClick = [this]
    {
        parameter.addKeyframe (clip.getCurrentPTS(), widget->getValue());
    };

    prev.onClick = [&]
    {
        auto prev = parameter.getPreviousKeyframeTime (clip.getCurrentPTS());
        player.setPosition (prev + clip.getStart() - clip.getOffset());
    };

    next.onClick = [&]
    {
        auto next = parameter.getNextKeyframeTime (clip.getCurrentPTS());
        player.setPosition (next + clip.getStart() - clip.getOffset());
    };

}

void ProcessorComponent::ParameterComponent::paint (Graphics& g)
{
    auto area = getLocalBounds().reduced (3);
    g.setColour (Colours::silver);
    g.drawFittedText (parameter.getName(), area, Justification::topLeft, 1);
}

void ProcessorComponent::ParameterComponent::resized()
{
    auto area = getLocalBounds().reduced (3);
    add.setBounds (area.removeFromRight (24).withTop (area.getHeight() - 24));
    next.setBounds (area.removeFromRight (24).withTop (area.getHeight() - 24));
    prev.setBounds (area.removeFromRight (24).withTop (area.getHeight() - 24));
    widget->getComponent().setBounds (area.withTop (20).withTrimmedRight (3));
}

void ProcessorComponent::ParameterComponent::updateForTime (double pts)
{
    widget->setValue (parameter.getValueForTime (pts));
}

//==============================================================================

ProcessorComponent::AudioProcessorWindow::AudioProcessorWindow (AudioProcessorEditor* editor, const String& title)
  : DocumentWindow (title, Colours::darkgrey, DocumentWindow::closeButton, true)
{
    setAlwaysOnTop (true);
    setWantsKeyboardFocus (false);
    setUsingNativeTitleBar (true);
    setResizable (editor->isResizable(), false);
    setContentOwned (editor, true);
    setVisible (true);
}

void ProcessorComponent::AudioProcessorWindow::closeButtonPressed()
{
    setVisible (false);
    setContentOwned (nullptr, false);
}
