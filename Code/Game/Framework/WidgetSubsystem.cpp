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
    for (Widget* widget : m_widgets)
    {
        delete widget;
        widget = nullptr;
    }
}

void WidgetSubsystem::BeginFrame()
{
    for (int i = 0; i < static_cast<int>(m_widgets.size()); ++i)
    {
        if (m_widgets[i] && m_widgets[i]->m_bIsGarbage)
        {
            delete m_widgets[i];
            m_widgets[i] = nullptr;
        }
    }

    for (Widget* widget : m_widgets)
    {
        if (widget)
            widget->BeginFrame();
    }
}

void WidgetSubsystem::Startup()
{
    printf("WidgetSubsystem::Startup    Initialize Widget Subsystem\n");
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
        if (widget && !widget->m_bIsGarbage)
            widget->Update();
    }
}

void WidgetSubsystem::Render()
{
    for (Widget* widget : m_widgets)
    {
        if (widget && !widget->m_bIsGarbage)
            widget->Render();
    }
}

void WidgetSubsystem::EndFrame()
{
    for (Widget* widget : m_widgets)
    {
        if (widget)
            widget->EndFrame();
    }
}

void WidgetSubsystem::AddToViewport(Widget* widget, int zOrder)
{
    printf("WidgetSubsystem::AddToViewport      Add widget %s\n", widget->GetName().c_str());
    widget->m_zOrder = zOrder;
    m_widgets.push_back(widget);
}

void WidgetSubsystem::AddToPlayerViewport(Widget* widget, PlayerController* player, int zOrder)
{
    printf("WidgetSubsystem::AddToPlayerViewport        Add widget %s to player viewport\n", widget->GetName().c_str());
    widget->m_zOrder = zOrder;
    widget->m_owner  = player;
    m_widgets.push_back(widget);
}

void WidgetSubsystem::RemoveFromViewport(Widget* widget)
{
    for (Widget* m_widget : m_widgets)
    {
        if (m_widget == widget)
        {
            printf("WidgetSubsystem::RemoveFromViewport        Remove widget %s\n", widget->GetName().c_str());
            m_widget->RemoveFromViewport();
        }
    }
}

void WidgetSubsystem::RemoveFromViewport(std::string widgetName)
{
    for (Widget* widget : m_widgets)
    {
        if (widget && widget->m_name == widgetName)
        {
            printf("WidgetSubsystem::RemoveFromViewport        Remove widget %s\n", widget->GetName().c_str());
            widget->RemoveFromViewport();
        }
    }
}

void WidgetSubsystem::RemoveFromPlayerViewport(PlayerController* player, std::string widgetName)
{
    for (Widget* widget : m_widgets)
    {
        if (widget && widget->m_name == widgetName && widget->m_owner == player)
        {
            printf("WidgetSubsystem::RemoveFromViewport        Remove player %d 's widget %s\n", player->m_index, widget->GetName().c_str());
            widget->RemoveFromViewport();
        }
    }
}
