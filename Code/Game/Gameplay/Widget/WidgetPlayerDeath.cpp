#include "WidgetPlayerDeath.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/Game.hpp"

WidgetPlayerDeath::WidgetPlayerDeath()
{
    m_name = "WidgetPlayerDeath";
}

void WidgetPlayerDeath::Draw() const
{
    Widget::Draw();
    g_theRenderer->SetBlendMode(BlendMode::ALPHA);
    g_theRenderer->BindTexture(nullptr);
    g_theRenderer->BindShader(nullptr);
    std::vector<Vertex_PCU> vertices;
    vertices.reserve(256);
    AddVertsForAABB2D(vertices, g_theGame->m_screenSpace, Rgba8(0, 0, 0, 140));
    g_theRenderer->DrawVertexArray(vertices);
}

void WidgetPlayerDeath::Update()
{
    Widget::Update();
}

WidgetPlayerDeath::~WidgetPlayerDeath()
{
}
