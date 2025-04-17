#pragma once
#include "Game/Framework/Widget.hpp"

class WidgetAttract : public Widget
{
public:
    WidgetAttract();
    ~WidgetAttract() override;

public:
    void Draw() const override;
    void Update() override;

    /// Event
    STATIC bool OnStateChange(EventArgs& args);

protected:
    void UpdateKeyInput();

private:
    float FluctuateValue(float value, float amplitude, float frequency, float deltaTime);

    float m_iconCircleRadius           = 200;
    float m_currentIconCircleThickness = 0.f;
    int   m_counter                    = 0;

    std::vector<Vertex_PCU> m_lowerInfoStringVertex;

    std::string m_lowerInfoString = "Press SPACE to join with mouse and keyboard\n"
        "Press START to koin with controller\n"
        "Press ESCAPE or BACK to exit\n";
};
