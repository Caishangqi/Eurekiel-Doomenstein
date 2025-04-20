#pragma once
#include "Game/Framework/Widget.hpp"

class WidgetPlayerDeath : public Widget
{
public:
    WidgetPlayerDeath();
    ~WidgetPlayerDeath() override;

    void Draw() const override;
    void Update() override;
};
