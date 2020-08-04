#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <bitset>

enum class DockAnchor
{
    Left,
    Top,
    Bottom,
    Right,
    Fill
};

class DockOverlay : public juce::Component
{
public:
    static constexpr int Overlay_Size = 70;
    static constexpr int Box_Margin   = 2;
    
    //==================================================================================================================
    DockOverlay() noexcept : anchors(31) {}
    
    DockOverlay(std::initializer_list<DockAnchor> shownAnchors)
    {
        for (auto &anchor : shownAnchors)
        {
            if (anchor != DockAnchor::Fill)
            {
                anchors[static_cast<int>(anchor)] = true;
            }
        }
    }
    
    //==================================================================================================================
    void paint(Graphics &g) override
    {
        if (!shouldDraw)
        {
            return;
        }
        
        g.setColour(juce::Colours::white);
        
        if (anchors[static_cast<int>(DockAnchor::Left)])
        {
            g.setOpacity(currentAnchor == DockAnchor::Left ? 0.3f : 0.1f);
            g.fillPath(pLeft);
        }
    
        if (anchors[static_cast<int>(DockAnchor::Top)])
        {
            g.setOpacity(currentAnchor == DockAnchor::Top ? 0.3f : 0.1f);
            g.fillPath(pTop);
        }
        
        if (anchors[static_cast<int>(DockAnchor::Right)])
        {
            g.setOpacity(currentAnchor == DockAnchor::Right ? 0.3f : 0.1f);
            g.fillPath(pRight);
        }
        
        if (anchors[static_cast<int>(DockAnchor::Bottom)])
        {
            g.setOpacity(currentAnchor == DockAnchor::Bottom ? 0.3f : 0.1f);
            g.fillPath(pBottom);
        }

        g.setOpacity(currentAnchor == DockAnchor::Fill ? 0.3f : 0.1f);
        g.fillRect(pCentre);
    }
    
    void resized() override
    {
        pLeft.clear();
        pLeft.addQuadrilateral(0,            Box_Margin,
                               Overlay_Size, Overlay_Size + Box_Margin,
                               Overlay_Size, getHeight()  - Overlay_Size - Box_Margin,
                               0,            getHeight()  - Box_Margin);
    
        pTop.clear();
        pTop.addQuadrilateral(Box_Margin,                               0,
                              getWidth()   - Box_Margin,                0,
                              getWidth()   - Overlay_Size - Box_Margin, Overlay_Size,
                              Overlay_Size + Box_Margin,                Overlay_Size);
    
        pRight.clear();
        pRight.addQuadrilateral(getWidth(),                Box_Margin,
                                getWidth(),                getHeight()  - Box_Margin,
                                getWidth() - Overlay_Size, getHeight()  - Overlay_Size - Box_Margin,
                                getWidth() - Overlay_Size, Overlay_Size + Box_Margin);
    
        pBottom.clear();
        pBottom.addQuadrilateral(Box_Margin,                               getHeight(),
                                 Overlay_Size + Box_Margin,                getHeight() - Overlay_Size,
                                 getWidth()   - Overlay_Size - Box_Margin, getHeight() - Overlay_Size,
                                 getWidth()   - Box_Margin,                getHeight());
        
        pCentre.setBounds(Overlay_Size + 2 * Box_Margin, Overlay_Size + 2 * Box_Margin,
                          getWidth()  - Overlay_Size * 2 - Box_Margin * 4,
                          getHeight() - Overlay_Size * 2 - Box_Margin * 4);
    }
    
    //==================================================================================================================
    void mouseMove(const juce::MouseEvent &event) override
    {
        DockAnchor last = currentAnchor;
        const auto &pos = event.position;
        
        if (pLeft.contains(pos) && anchors[static_cast<int>(DockAnchor::Left)])
        {
            currentAnchor = DockAnchor::Left;
        }
        else if (pTop.contains(pos) && anchors[static_cast<int>(DockAnchor::Top)])
        {
            currentAnchor = DockAnchor::Top;
        }
        else if (pRight.contains(pos) && anchors[static_cast<int>(DockAnchor::Right)])
        {
            currentAnchor = DockAnchor::Right;
        }
        else if (pBottom.contains(pos) && anchors[static_cast<int>(DockAnchor::Bottom)])
        {
            currentAnchor = DockAnchor::Bottom;
        }
        else if (pCentre.contains(pos.toInt()))
        {
            currentAnchor = DockAnchor::Fill;
        }
    
        if (last != currentAnchor)
        {
            repaint();
        }
    }
    
    void mouseEnter(const juce::MouseEvent&) override
    {
        shouldDraw = true;
        repaint();
    }
    
    void mouseExit(const juce::MouseEvent&) override
    {
        shouldDraw = false;
        repaint();
    }
    
    //==================================================================================================================
    void enableAnchor(DockAnchor anchor, bool enable) noexcept
    {
        if (anchor != DockAnchor::Fill)
        {
            anchors[static_cast<int>(anchor)] = enable;
            repaint();
        }
    }
    
    bool isAnchorEnabled(DockAnchor anchor) const noexcept
    {
        return anchors[static_cast<int>(anchor)];
    }
    
    //==================================================================================================================
    DockAnchor getCurrentDockAnchor() const noexcept
    {
        return currentAnchor;
    }
    
private:
    juce::Path pLeft;
    juce::Path pTop;
    juce::Path pRight;
    juce::Path pBottom;
    juce::Rectangle<int> pCentre;
    
    std::bitset<4> anchors { 0 };
    DockAnchor currentAnchor { DockAnchor::Fill };
    
    bool shouldDraw { false };
};

class DockingContainer;
class DockingPanel : public juce::TabbedComponent
{
public:
    DockingPanel(DockingContainer &container, int id) noexcept
        : TabbedComponent(juce::TabbedButtonBar::Orientation::TabsAtTop),
          container(container), id(id)
    {
        addAndMakeVisible(content);
        addChildComponent(overlay);
    }
    
    //==================================================================================================================
    void paint(Graphics &g) override
    {
        g.setColour(juce::Colours::pink);
        g.fillAll();
    }
    
    void resized() override
    {
        content.setBounds(getLocalBounds());
        overlay.setBounds(getLocalBounds());
    }
    
    //==================================================================================================================
    int getId() const noexcept { return id; }
    
    //==================================================================================================================
    void unpin()
    {
        if (juce::Component *parent; !dynamic_cast<juce::DialogWindow*>(parent = getParentComponent()))
        {
            parent->removeChildComponent(this);
            
            DialogWindow::LaunchOptions options;
            options.content.set(this, false);
            options.dialogTitle = "Test";
            options.useNativeTitleBar = true;
            options.escapeKeyTriggersCloseButton = false;
            options.resizable = true;
            
            options.launchAsync();
        }
    }
    
    //==================================================================================================================
    juce::Component& getContentComponent() noexcept
    {
        return content;
    }
    
    bool isWindowed() const
    {
        return dynamic_cast<juce::DialogWindow*>(getParentComponent());
    }
    
    //==================================================================================================================
    void enableAnchor(DockAnchor anchor, bool enabled) noexcept
    {
        overlay.enableAnchor(anchor, enabled);
    }
    
    bool isAnchorEnabled(DockAnchor anchor) const noexcept
    {
        return overlay.isAnchorEnabled(anchor);
    }
    
    //==================================================================================================================
    bool autoResisesX() const noexcept
    {
        return resisesX;
    }
    
    bool autoResisesY() const noexcept
    {
        return resisesY;
    }
    
private:
    DockingContainer &container;
    DockOverlay overlay;
    juce::Component content;
    int id;
    
    bool resisesX;
    bool resisesY;
};


class SplitContainer : public juce::Component
{
public:
    enum Direction
    {
        Vertical,
        Horizontal
    };
    
    //==================================================================================================================
    explicit SplitContainer(Direction direction = Horizontal) noexcept
        : direction(direction)
    {
        addAndMakeVisible(firstContainer);
        addAndMakeVisible(resiser);
        addAndMakeVisible(secondContainer);
    }
    
    //==================================================================================================================
    void paint(Graphics &g) override
    {
        g.setColour(juce::Colours::black);
        g.fillRect(resiser.getBounds());
    }
    
    void resized() override
    {
        if (direction == Horizontal)
        {
            int new_x = 0;
            
            if (component1 && component2)
            {
                if (int diff = getWidth() - prevSize.getWidth(); diff != 0)
                {
                    new_x = resiser.getX() + (getWidth() - prevSize.getWidth()) / 2 - thickness / 2;
                }
            }
            else
            {
                new_x = component1 ? getWidth() : -thickness;
            }
            
            resiser        .setBounds(new_x,              0, thickness,                       getHeight());
            firstContainer .setBounds(0,                  0, resiser.getX(),                  getHeight());
            secondContainer.setBounds(resiser.getRight(), 0, getWidth() - resiser.getRight(), getHeight());
        }
        else
        {
            int new_y = 0;
    
            if (component1 && component2)
            {
                if (int diff = getWidth() - prevSize.getWidth(); diff != 0)
                {
                    new_y = resiser.getY() + (getHeight() - prevSize.getHeight()) / 2;
                }
            }
            else
            {
                new_y = component1 ? getHeight() : -thickness;
            }

            resiser        .setBounds(0, new_y,               getWidth(), thickness);
            firstContainer .setBounds(0, 0,                   getWidth(), resiser.getY());
            secondContainer.setBounds(0, resiser.getBottom(), getWidth(), getHeight() - resiser.getBottom());
        }
        
        if (firstContainer.getNumChildComponents() > 0)
        {
            component1->setBounds(firstContainer.getBounds());
        }
    
        if (secondContainer.getNumChildComponents() > 0)
        {
            component2->setBounds(secondContainer.getBounds());
        }
        
        prevSize = getLocalBounds();
    }
    
    //==================================================================================================================
    void setTopOrLeftComponent(juce::Component *const component) noexcept
    {
        if (component == component1)
        {
            return;
        }
        
        if (component && !component1 && component2)
        {
            if (direction == Horizontal)
            {
                resiser.setBounds(getWidth() / 2 - thickness / 2, 0, thickness, getHeight());
            }
            else
            {
                resiser.setBounds(0, getHeight() / 2 - thickness / 2, getWidth(), thickness);
            }
        }
        
        firstContainer.removeAllChildren();
        firstContainer.addAndMakeVisible(component1 = component);
        resized();
    }
    
    void setBottomOrRightComponent(juce::Component *const component) noexcept
    {
        if (component == component2)
        {
            return;
        }
    
        if (component && component1 && !component2)
        {
            if (direction == Horizontal)
            {
                resiser.setBounds(getWidth() / 2 - thickness / 2, 0, thickness, getHeight());
            }
            else
            {
                resiser.setBounds(0, getHeight() / 2 - thickness / 2, getWidth(), thickness);
            }
        }
        
        secondContainer.removeAllChildren();
        secondContainer.addAndMakeVisible(component2 = component);
        resized();
    }
    
    //==================================================================================================================
    void setResiserThickness(int thickness) noexcept
    {
        this->thickness = thickness;
        resized();
        repaint();
    }
    
private:
    juce::Component *component1 {};
    juce::Component *component2 {};
    
    SplitResiser resiser;
    
    juce::Component firstContainer;
    juce::Component secondContainer;
    
    juce::Rectangle<int> prevSize;
    
    Direction direction;
    int thickness { 3 };
};

template<class T>
struct Thickness
{
    T left;
    T top;
    T right;
    T bottom;
};

class DockingContainer : public juce::Component
{
public:
    explicit DockingContainer()
    {
        addChildComponent(overlay);
    }
    
    //==================================================================================================================
    void paint(Graphics &g) override
    {
        g.setColour(juce::Colours::blueviolet);
        g.fillAll();
    }
    
    void resized() override
    {
        
        
        overlay.setBounds(getLocalBounds());
    }
    
    //==================================================================================================================
    void setContainerPadding(Thickness<int> padding) noexcept
    {
        containerPadding = padding;
    }
    
    void setPanelMargin(Thickness<int> margin) noexcept
    {
        panelMargin = margin;
    }
    
    //==================================================================================================================
    void enableAnchor(DockAnchor anchor, bool enabled) noexcept
    {
        overlay.enableAnchor(anchor, enabled);
    }
    
    bool isAnchorEnabled(DockAnchor anchor) const noexcept
    {
        return overlay.isAnchorEnabled(anchor);
    }
    
private:
    struct Dock
    {
        DockingPanel panel;
        DockAnchor anchor;
    };
    
    std::vector<std::vector<DockingPanel*>> panelMatrix;
    
    Thickness<int> containerPadding { 3, 3, 3, 3 };
    Thickness<int> panelMargin      { 3, 3, 3, 3 };
    
    DockOverlay overlay;
};
