#pragma once
#include <set>
#include <string>

#include "Engine/Core/Clock.hpp"

class Widget;
class PlayerController;

struct WidgetSystemConfig
{
    Clock*      m_clock       = nullptr;
    std::string m_defaultName = "Untitled";
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

protected:
    void AddToViewport(Widget* widget, int zOrder = 0);
    void AddToPlayerViewport(Widget* widget, PlayerController* player, int zOrder = 0);

private:
    WidgetSystemConfig                     m_config;
    std::set<Widget*, DescendingZOrderPtr> m_widgets;

private:
};
