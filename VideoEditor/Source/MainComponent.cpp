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

    Main GUI for the components for the VideoEditor

  ==============================================================================
*/

#include "MainComponent.h"
#include "RenderDialog.h"
#include "ColourDictionary.h"

namespace CommandIDs
{
    enum
    {
        fileOpen = 100,
        fileSave,
        fileSaveAs,
        fileNew,
        fileRender,
        fileQuit,

        editPreferences = 200,
        editSplice,
        editVisibility,

        playStart = 300,
        playStop,
        playReturn,
        playRecord,

        trackAdd = 400,
        trackRemove,

        viewFullScreen = 500,
        viewExitFullScreen,

        helpAbout = 600,
        helpHelp
    };
}

//==============================================================================
MainComponent::MainComponent()
    : multiTabPane(jaut::MultiPagePane::TabFactoryTemplates::Default::create(
          [](const juce::String &id) -> std::unique_ptr<juce::Component>
          {
              if (id == "button")
              {
                  return std::make_unique<juce::TextButton>("SUM TEXT");
              }
              else if (id == "textbox")
              {
                  return std::make_unique<juce::TextEditor>();
              }
                
              return nullptr;
           })
      )
{
//    levelMeter.setLookAndFeel (&lookAndFeel);
//    lookAndFeel.setColour (FFAU::LevelMeter::lmBackgroundColour, getLookAndFeel().findColour (ResizableWindow::backgroundColourId).darker());
//    lookAndFeel.setColour (FFAU::LevelMeter::lmTicksColour, Colours::silver);

    //addAndMakeVisible (library);
    //addAndMakeVisible (preview);
    //addAndMakeVisible (properties);
    //addAndMakeVisible (viewport);
    //addAndMakeVisible (transport);
    //addAndMakeVisible (levelMeter);

    multiTabPane.addPage("button",  "A button");
    multiTabPane.addPage("textbox", "A textbox");
    
    jaut::MultiPagePane::Style style;
    //style.tabBarLayout = jaut::MultiPagePane::Style::TabBarLayout::SideLeft;
    
    jaut::MultiPagePane::Options options;
    options.pinnedTabBehaviour |= jaut::MultiPagePane::Options::PinBehaviour::PutInExtraRow;
    options.allowTabReordering =  true;
    
    multiTabPane.setStyle(style);
    multiTabPane.setOptions(options);
    addAndMakeVisible(multiTabPane);
    
    viewport.setViewedComponent (&timeline);
    timeline.setSize (2000, 510);

    const auto area = juce::Desktop::getInstance().getDisplays().getMainDisplay().userArea;
    setBounds (area);

    player.initialise();
    lmLookAndFeel.setColour (foleys::LevelMeter::lmBackgroundColour, getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId).darker());
    lmLookAndFeel.setColour (foleys::LevelMeter::lmOutlineColour, juce::Colours::transparentBlack);
    lmLookAndFeel.setColour (foleys::LevelMeter::lmMeterOutlineColour, juce::Colours::transparentBlack);
    lmLookAndFeel.setColour (foleys::LevelMeter::lmTicksColour, juce::Colours::silver);
    levelMeter.setLookAndFeel (&lmLookAndFeel);
    levelMeter.setMeterSource (&player.getMeterSource());
    resetEdit();

    juce::LookAndFeel& lf = getLookAndFeel();
    lf.setColour(AppColours::WindowBackgroundId, juce::Colour::fromString("#17223b"));
    lf.setColour(AppColours::DockerBackgroundId, juce::Colour::fromString("#6b778d"));

    commandManager.registerAllCommandsForTarget (this);
    commandManager.setFirstCommandTarget (this);
#if JUCE_MAC
    setMacMainMenu (this);
#endif

    commandManager.getKeyMappings()->resetToDefaultMappings();

    auto settingsFolder = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile (PROJECT_INFO_COMPANY).getChildFile (PROJECT_INFO_NAME);
    settingsFolder.createDirectory();
    videoEngine.getAudioPluginManager().setPluginDataFile (settingsFolder.getChildFile ("PluginList.xml"));

    startTimerHz (10);
}

MainComponent::~MainComponent()
{
    if (auto edit = timeline.getEditClip())
        edit->removeTimecodeListener (&preview);

#if JUCE_MAC
    setMacMainMenu (nullptr);
#endif

    levelMeter.setLookAndFeel (nullptr);
}

juce::KeyPressMappingSet* MainComponent::getKeyMappings() const
{
    return commandManager.getKeyMappings();
}

//==============================================================================

void MainComponent::paint (juce::Graphics& g)
{
    const juce::LookAndFeel& lf = getLookAndFeel();
    
    g.fillAll (lf.findColour (AppColours::WindowBackgroundId));
    
    g.setColour (lf.findColour (juce::ResizableWindow::backgroundColourId).darker());
    g.fillRect (getLocalBounds().withTop (lowerPart));
    
    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds());
}

void MainComponent::resized()
{
    if (viewerFullScreen)
    {
        preview.setBounds (getLocalBounds());
        preview.toFront (false);
    }
    else
    {
        auto bounds = getLocalBounds().reduced (1);
        //lowerPart = juce::roundToInt (bounds.getHeight() * 0.4);
        auto lower  = bounds.removeFromBottom (lowerPart);
        //levelMeter.setBounds (lower.removeFromRight (120).reduced (2));
        //lower.removeFromTop (14); // TODO: ruler
        //viewport.setBounds (lower);
        auto sides = juce::roundToInt (bounds.getWidth() / 4.0);
        //library.setBounds (bounds.removeFromLeft (sides));
        //properties.setBounds (bounds.removeFromRight (sides));
        //transport.setBounds (bounds.removeFromBottom (24));
        //previewContainer.setBounds (bounds);
    }
    
    multiTabPane.setBounds(getLocalBounds().withTrimmedRight(500));
}

void MainComponent::resetEdit()
{
    auto edit = std::make_shared<foleys::ComposedClip> (videoEngine);
    videoEngine.manageLifeTime (edit);

    timeline.setEditClip (edit);
    edit->addTimecodeListener (&preview);
    editFileName = juce::File();
    updateTitleBar();

    videoEngine.getUndoManager()->clearUndoHistory();
}

void MainComponent::loadEdit()
{
    juce::FileChooser myChooser ("Please select the project you want to save...",
                                 juce::File::getSpecialLocation (juce::File::userMoviesDirectory),
                                 "*.videdit");
    if (myChooser.browseForFileToOpen())
    {
        loadEditFile (myChooser.getResult());
    }
}

void MainComponent::loadEditFile (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml.get() == nullptr)
    {
        juce::AlertWindow::showMessageBox (juce::AlertWindow::WarningIcon,
                                           NEEDS_TRANS ("Loading failed"),
                                           "Loading of the file \"" + file.getFullPathName() + "\" failed.");
        return;
    }

    editFileName = file;
    auto tree = juce::ValueTree::fromXml (*xml);
    auto edit = std::make_shared<foleys::ComposedClip>(videoEngine);
    videoEngine.manageLifeTime (edit);

    for (auto clip : tree)
        edit->getStatusTree().appendChild (clip.createCopy(), nullptr);

    timeline.setEditClip (edit);
    edit->addTimecodeListener (&preview);

    player.setPosition (0);
    updateTitleBar();

    videoEngine.getUndoManager()->clearUndoHistory();

    timeline.resized();
}

void MainComponent::saveEdit (bool saveAs)
{
    auto edit = timeline.getEditClip();

    if (saveAs || editFileName.getFullPathName().isEmpty())
    {
        juce::FileChooser myChooser ("Please select the project you want to save...",
                                     juce::File::getSpecialLocation (juce::File::userMoviesDirectory),
                                     "*.videdit");
        if (myChooser.browseForFileToSave (true))
        {
            editFileName = myChooser.getResult();
        }
        else
        {
            return;
        }
    }

    if (edit && editFileName.getFullPathName().isNotEmpty())
    {
        juce::FileOutputStream output (editFileName);
        if (output.openedOk())
        {
            edit->readPluginStatesIntoValueTree();

            output.setPosition (0);
            output.truncate();
            output.writeString (edit->getStatusTree().toXmlString());
        }
        else
        {
            juce::AlertWindow::showMessageBox (juce::AlertWindow::WarningIcon, NEEDS_TRANS("Saving failed"), "Saving of file \"" + editFileName.getFullPathName() + "\" failed.");
        }
        updateTitleBar();
    }
}

bool MainComponent::handleQuitRequest()
{
    if (renderer.isRendering())
    {
        if (juce::AlertWindow::showOkCancelBox (juce::AlertWindow::WarningIcon, NEEDS_TRANS("Quit application"), "Cancel rendering to quit?"))
            renderer.cancelRendering();
        else
            return false;
    }

    return true;
}

void MainComponent::showRenderDialog()
{
    if (! renderer.isRendering())
        renderer.setClipToRender (timeline.getEditClip()->createCopy (foleys::StreamTypes::all()));

    properties.showProperties (std::make_unique<RenderDialog>(renderer));
}

void MainComponent::deleteSelectedClip()
{
    if (auto selected = timeline.getSelectedClip())
        if (auto edit = timeline.getEditClip())
            edit->removeClip (selected);
}

void MainComponent::showPreferences()
{
    auto selector = std::make_unique<juce::AudioDeviceSelectorComponent>(deviceManager, 0, 2, 2, 2, false, false, true, false);
    properties.showProperties (std::move (selector));
}

void MainComponent::updateTitleBar()
{
    if (auto* window = dynamic_cast<juce::DocumentWindow*>(juce::TopLevelWindow::getActiveTopLevelWindow()))
    {
        if (editFileName.getFullPathName().isNotEmpty())
            window->setName (PROJECT_INFO_NAME + juce::String (": ") + editFileName.getFileNameWithoutExtension());
        else
            window->setName (PROJECT_INFO_NAME);
    }
}

void MainComponent::setViewerFullScreen (bool shouldBeFullScreen)
{
    viewerFullScreen = shouldBeFullScreen;
    resized();
}

//==============================================================================

void MainComponent::getAllCommands (juce::Array<juce::CommandID>& commands)
{
    commands.add (CommandIDs::fileNew, CommandIDs::fileOpen, CommandIDs::fileSave, CommandIDs::fileSaveAs, CommandIDs::fileRender, juce::StandardApplicationCommandIDs::quit);
    commands.add (juce::StandardApplicationCommandIDs::undo, juce::StandardApplicationCommandIDs::redo,
                  juce::StandardApplicationCommandIDs::del, juce::StandardApplicationCommandIDs::copy, juce::StandardApplicationCommandIDs::paste,
                  CommandIDs::editSplice, CommandIDs::editVisibility, CommandIDs::editPreferences);
    commands.add (CommandIDs::playStart, CommandIDs::playStop, CommandIDs::playReturn);
    commands.add (CommandIDs::trackAdd, CommandIDs::trackRemove);
    commands.add (CommandIDs::viewFullScreen, CommandIDs::viewExitFullScreen);
    commands.add (CommandIDs::helpAbout, CommandIDs::helpHelp);
}

void MainComponent::getCommandInfo (juce::CommandID commandID, juce::ApplicationCommandInfo& result)
{
    auto categoryFile  = "file";
    auto categoryEdit  = "edit";
    auto categoryPlay  = "play";
    auto categoryTrack = "track";
    auto categoryView  = "view";
    auto categoryHelp  = "help";

    switch (commandID)
    {
        case CommandIDs::fileNew:
            result.setInfo ("New Project...", "Clear all and start fresh", categoryFile, 0);
            result.defaultKeypresses.add (juce::KeyPress ('n', juce::ModifierKeys::commandModifier, 0));
            break;
        case CommandIDs::fileOpen:
            result.setInfo ("Open Project...", "Select a project to open", categoryFile, 0);
            result.defaultKeypresses.add (juce::KeyPress ('o', juce::ModifierKeys::commandModifier, 0));
            break;
        case CommandIDs::fileSave:
            result.setInfo ("Save Project", "Save the current project", categoryFile, 0);
            result.defaultKeypresses.add (juce::KeyPress ('s', juce::ModifierKeys::commandModifier, 0));
            break;
        case CommandIDs::fileSaveAs:
            result.setInfo ("Save Project...", "Save the current project as new file", categoryFile, 0);
            break;
        case CommandIDs::fileRender:
            result.setInfo ("Render Project...", "Export the piece into an audio file", categoryFile, 0);
            result.defaultKeypresses.add (juce::KeyPress ('r', juce::ModifierKeys::commandModifier, 0));
            break;
        case juce::StandardApplicationCommandIDs::quit:
            result.setInfo ("Quit...", "Quit Application", categoryFile, 0);
            result.defaultKeypresses.add (juce::KeyPress ('q', juce::ModifierKeys::commandModifier, 0));
            break;
        case juce::StandardApplicationCommandIDs::undo:
            result.setInfo ("Undo", "Undo the last step", categoryEdit, 0);
            result.defaultKeypresses.add (juce::KeyPress ('z', juce::ModifierKeys::commandModifier, 0));
            break;
        case juce::StandardApplicationCommandIDs::redo:
            result.setInfo ("Redo", "Redo the last undo step", categoryEdit, 0);
            result.defaultKeypresses.add (juce::KeyPress ('z', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier, 0));
            break;
        case juce::StandardApplicationCommandIDs::del:
            result.setInfo ("Delete", "Delete the selected gesture", categoryEdit, 0);
            result.defaultKeypresses.add (juce::KeyPress (juce::KeyPress::backspaceKey, juce::ModifierKeys::noModifiers, 0));
            break;
        case juce::StandardApplicationCommandIDs::copy:
            result.setInfo ("Copy", "Copy the selected gesture", categoryEdit, 0);
            result.defaultKeypresses.add (juce::KeyPress ('c', juce::ModifierKeys::commandModifier, 0));
            break;
        case juce::StandardApplicationCommandIDs::paste:
            result.setInfo ("Paste", "Paste the gesture in the clipboard", categoryEdit, 0);
            result.defaultKeypresses.add (juce::KeyPress ('v', juce::ModifierKeys::commandModifier, 0));
            break;
        case CommandIDs::editSplice:
            result.setInfo ("Splice", "Split selected clip at play position", categoryEdit, 0);
            result.defaultKeypresses.add (juce::KeyPress ('b', juce::ModifierKeys::commandModifier, 0));
            break;
        case CommandIDs::editVisibility:
            result.setInfo ("Visible", "Toggle visibility or mute of the selected clip", categoryEdit, 0);
            result.defaultKeypresses.add (juce::KeyPress ('v', juce::ModifierKeys::noModifiers, 0));
            break;
        case CommandIDs::editPreferences:
            result.setInfo ("Preferences", "Open the audio preferences", categoryEdit, 0);
            result.defaultKeypresses.add (juce::KeyPress (',', juce::ModifierKeys::commandModifier, 0));
            break;
        case CommandIDs::playStart:
            result.setInfo ("Play", "Start/Pause playback", categoryPlay, 0);
            result.defaultKeypresses.add (juce::KeyPress (juce::KeyPress::spaceKey, juce::ModifierKeys::noModifiers, 0));
            break;
        case CommandIDs::playStop:
            result.setInfo ("Stop", "Stop playback", categoryPlay, 0);
            break;
        case CommandIDs::playReturn:
            result.setInfo ("Return", "Set playhead to begin", categoryPlay, 0);
            result.defaultKeypresses.add (juce::KeyPress (juce::KeyPress::returnKey, juce::ModifierKeys::noModifiers, 0));
            break;
        case CommandIDs::trackAdd:
            result.setInfo ("Add Track", "Add a new AUX track", categoryTrack, 0);
            result.defaultKeypresses.add (juce::KeyPress ('t', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier, 0));
            break;
        case CommandIDs::trackRemove:
            result.setInfo ("Remove Track", "Remove an AUX track", categoryTrack, 0);
            break;
        case CommandIDs::viewFullScreen:
            result.setInfo ("Fullscreen", "Maximise Viewer", categoryView, 0);
            result.defaultKeypresses.add (juce::KeyPress (juce::KeyPress::returnKey, juce::ModifierKeys::commandModifier, 0));
            break;
        case CommandIDs::viewExitFullScreen:
            result.setInfo ("Exit Fullscreen", "Normal viewer size", categoryView, 0);
            result.defaultKeypresses.add (juce::KeyPress (juce::KeyPress::escapeKey, juce::ModifierKeys::noModifiers, 0));
            break;
        case CommandIDs::helpAbout:
            result.setInfo ("About", "Show information about the program", categoryHelp, 0);
            break;
        case CommandIDs::helpHelp:
            result.setInfo ("Help", "Show help how to use the program", categoryHelp, 0);
            break;
        default:
            juce::JUCEApplication::getInstance()->getCommandInfo (commandID, result);
            break;
    }
}

bool MainComponent::perform (const InvocationInfo& info)
{
    switch (info.commandID) {
        case CommandIDs::fileNew: resetEdit(); break;
        case CommandIDs::fileOpen: loadEdit(); break;
        case CommandIDs::fileSave: saveEdit(false); break;
        case CommandIDs::fileSaveAs: saveEdit(true); break;
        case CommandIDs::fileRender: showRenderDialog(); break;
        case juce::StandardApplicationCommandIDs::quit: juce::JUCEApplication::getInstance()->systemRequestedQuit(); break;

        case juce::StandardApplicationCommandIDs::undo: videoEngine.getUndoManager()->undo(); break;
        case juce::StandardApplicationCommandIDs::redo: videoEngine.getUndoManager()->redo(); break;
        case juce::StandardApplicationCommandIDs::del: deleteSelectedClip(); break;
        case CommandIDs::editSplice: timeline.spliceSelectedClipAtPlayPosition(); break;
        case CommandIDs::editVisibility: timeline.toggleVisibility(); break;

        case CommandIDs::editPreferences: showPreferences(); break;

        case CommandIDs::playStart: if (player.isPlaying()) player.stop(); else player.start(); break;
        case CommandIDs::playStop: player.stop(); break;
        case CommandIDs::playReturn: player.setPosition (0.0) ; break;

        case CommandIDs::trackAdd: break;
        case CommandIDs::trackRemove: break;

        case CommandIDs::viewFullScreen: setViewerFullScreen (! viewerFullScreen); break;
        case CommandIDs::viewExitFullScreen: setViewerFullScreen (false); break;

        default:
            jassertfalse;
            break;
    }
    return true;
}

juce::StringArray MainComponent::getMenuBarNames()
{
    return {NEEDS_TRANS ("File"), NEEDS_TRANS ("Edit"), NEEDS_TRANS ("Play"), NEEDS_TRANS ("Track"), NEEDS_TRANS ("View"), NEEDS_TRANS ("Help")};
}

juce::PopupMenu MainComponent::getMenuForIndex (int topLevelMenuIndex,
                                                const juce::String&)
{
    juce::PopupMenu menu;
    if (topLevelMenuIndex == 0)
    {
        menu.addCommandItem (&commandManager, CommandIDs::fileNew);
        menu.addSeparator();
        menu.addCommandItem (&commandManager, CommandIDs::fileOpen);
        menu.addCommandItem (&commandManager, CommandIDs::fileSave);
        menu.addCommandItem (&commandManager, CommandIDs::fileSaveAs);
        menu.addSeparator();
        menu.addCommandItem (&commandManager, CommandIDs::fileRender);
#if ! JUCE_MAC
        menu.addSeparator();
        menu.addCommandItem (&commandManager, juce::StandardApplicationCommandIDs::quit);
#endif
    }
    else if (topLevelMenuIndex == 1)
    {
        menu.addCommandItem (&commandManager, juce::StandardApplicationCommandIDs::undo);
        menu.addCommandItem (&commandManager, juce::StandardApplicationCommandIDs::redo);
        menu.addSeparator();
        menu.addCommandItem (&commandManager, juce::StandardApplicationCommandIDs::del);
        menu.addCommandItem (&commandManager, juce::StandardApplicationCommandIDs::copy);
        menu.addCommandItem (&commandManager, juce::StandardApplicationCommandIDs::paste);
        menu.addSeparator();
        menu.addCommandItem (&commandManager, CommandIDs::editSplice);
        menu.addCommandItem (&commandManager, CommandIDs::editVisibility);
        menu.addSeparator();
        menu.addCommandItem (&commandManager, CommandIDs::editPreferences);
    }
    else if (topLevelMenuIndex == 2)
    {
        menu.addCommandItem (&commandManager, CommandIDs::playStart);
        menu.addCommandItem (&commandManager, CommandIDs::playStop);
        menu.addCommandItem (&commandManager, CommandIDs::playReturn);
    }
    else if (topLevelMenuIndex == 3)
    {
        menu.addCommandItem (&commandManager, CommandIDs::trackAdd);
        menu.addCommandItem (&commandManager, CommandIDs::trackRemove);
    }
    else if (topLevelMenuIndex == 4)
    {
        menu.addCommandItem (&commandManager, CommandIDs::viewFullScreen);
        menu.addCommandItem (&commandManager, CommandIDs::viewExitFullScreen);
    }
    else if (topLevelMenuIndex == 5)
    {
        menu.addCommandItem (&commandManager, CommandIDs::helpAbout);
        menu.addCommandItem (&commandManager, CommandIDs::helpHelp);
    }
    return menu;
}

void MainComponent::timerCallback()
{
    for (auto& source : juce::Desktop::getInstance().getMouseSources())
        if (source.isDragging())
            return;

    videoEngine.getUndoManager()->beginNewTransaction();
}
