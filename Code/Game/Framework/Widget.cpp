#include "Widget.hpp"

#include "WidgetSubsystem.hpp"
#include "Game/GameCommon.hpp"

Widget::Widget()
{
}

Widget::~Widget()
{
}

void Widget::BeginFrame()
{
}

void Widget::Render()
{
}

void Widget::Update()
{
}

void Widget::EndFrame()
{
}

PlayerController* Widget::GetOwner()
{
    return m_owner;
}

int Widget::GetZOrder() const
{
    return m_zOrder;
}

void Widget::AddToViewport(int zOrder)
{
    g_theWidgetSubsystem->AddToViewport(this, zOrder);
}

void Widget::AddToPlayerViewport(PlayerController* player, int zOrder)
{
    g_theWidgetSubsystem->AddToPlayerViewport(this, player, zOrder);
}

void Widget::RemoveFromViewport()
{
}
