#include "WidgetSubsystem.hpp"

#include "Widget.hpp"

bool DescendingZOrderPtr::operator()(const Widget* lhs, const Widget* rhs) const
{
    // If two pointers point to the same object, they are considered equivalent
    if (lhs == rhs)
    {
        return false;
    }
    // If the zorder is the same, you need to define whether they are considered equal in the set.
    // Here, in order to meet the strict weak ordering (Strict Weak Ordering), when the zorder is the same,
    // We compare the pointers themselves to ensure that equal elements are not considered unequal in the set.
    if (lhs->GetZOrder() == rhs->GetZOrder())
    {
        return lhs < rhs;
    }
    // Larger zorders are placed on the "left" (i.e. keys that are considered smaller in the collection)
    return lhs->GetZOrder() > rhs->GetZOrder();
}

WidgetSubsystem::WidgetSubsystem(WidgetSystemConfig config): m_config(config)
{
}

WidgetSubsystem::~WidgetSubsystem()
{
}

void WidgetSubsystem::BeginFrame()
{
    for (Widget* widget : m_widgets)
    {
        widget->BeginFrame();
    }
}

void WidgetSubsystem::Startup()
{
    printf("WidgetSubsystem::Startup    Initialize widget subsystem\n");
}

void WidgetSubsystem::Shutdown()
{
    printf("WidgetSubsystem::Shutdown\n");
    for (Widget* widget : m_widgets)
    {
        delete widget;
        widget = nullptr;
    }
    m_widgets.clear();
}

void WidgetSubsystem::Update()
{
    for (Widget* widget : m_widgets)
    {
        widget->Update();
    }
}

void WidgetSubsystem::Render()
{
    for (Widget* widget : m_widgets)
    {
        widget->Render();
    }
}

void WidgetSubsystem::EndFrame()
{
    for (Widget* widget : m_widgets)
    {
        widget->EndFrame();
    }
}

void WidgetSubsystem::AddToViewport(Widget* widget, int zOrder)
{
    widget->m_zOrder = zOrder;
    m_widgets.insert(widget);
}

void WidgetSubsystem::AddToPlayerViewport(Widget* widget, PlayerController* player, int zOrder)
{
    widget->m_zOrder = zOrder;
    widget->m_owner  = player;
    m_widgets.insert(widget);
}
