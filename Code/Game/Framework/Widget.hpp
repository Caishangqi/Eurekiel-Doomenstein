#pragma once
#include "PlayerController.hpp"

class Widget
{
    friend class WidgetSubsystem;

public:
    Widget();
    virtual ~Widget();

public:
    virtual void BeginFrame();
    virtual void Render();
    virtual void Update();
    virtual void EndFrame();

    /// Getter
    virtual PlayerController* GetOwner();
    virtual int               GetZOrder() const;

public:
    virtual void AddToViewport(int zOrder = 0);
    virtual void AddToPlayerViewport(PlayerController* player, int zOrder = 0);
    virtual void RemoveFromViewport();

private:
    PlayerController* m_owner   = nullptr; // If player controller is null it basic means that it is the viewport widget.
    int               m_zOrder  = 0;
    bool              m_bIsTick = true;
};
