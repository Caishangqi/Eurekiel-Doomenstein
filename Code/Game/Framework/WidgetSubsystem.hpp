#pragma once
#include <set>
#include <string>

#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/Camera.hpp"

class Widget;
class PlayerController;

struct WidgetSystemConfig
{
    Clock*      m_clock          = nullptr;
    std::string m_defaultName    = "Untitled";
    Camera*     m_viewportCamera = nullptr;
};

struct DescendingZOrderPtr
{
    bool operator()(const Widget* lhs, const Widget* rhs) const;
};

class WidgetSubsystem
{
    friend class Widget;

public:
    WidgetSubsystem() = delete;
    WidgetSubsystem(WidgetSystemConfig config);
    ~WidgetSubsystem();

    void BeginFrame();

    void Startup();
    void Shutdown();
    void Update();
    void Render();

    void EndFrame();

public:
    void AddToViewport(Widget* widget, int zOrder = 0);
    void AddToPlayerViewport(Widget* widget, PlayerController* player, int zOrder = 0);
    void RemoveFromViewport(Widget* widget);
    // I certainly need an reflect system to remove the widget by class
    void RemoveFromViewport(std::string widgetName);

private:
    WidgetSystemConfig   m_config;
    std::vector<Widget*> m_widgets;
    //std::set<Widget*, DescendingZOrderPtr> m_widgets;

private:
};
