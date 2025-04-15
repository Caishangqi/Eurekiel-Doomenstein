#include "Hud.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Game/GameCommon.hpp"

Hud::Hud(XmlElement const& hudElement)
{
    m_shaderName           = ParseXmlAttribute(hudElement, "shader", m_shaderName);
    m_shader               = g_theRenderer->CreateShaderFromFile(m_shaderName.c_str(), VertexType::Vertex_PCU);
    m_reticleSize          = ParseXmlAttribute(hudElement, "reticleSize", m_reticleSize);
    m_spriteSize           = ParseXmlAttribute(hudElement, "spriteSize", m_spriteSize);
    m_spritePivot          = ParseXmlAttribute(hudElement, "spritePivot", m_spritePivot);
    m_baseTexturePath      = ParseXmlAttribute(hudElement, "baseTexture", m_baseTexturePath);
    m_m_reticleTexturePath = ParseXmlAttribute(hudElement, "reticleTexture", m_m_reticleTexturePath);

    m_baseTexture    = g_theRenderer->CreateTextureFromFile(m_baseTexturePath.c_str());
    m_reticleTexture = g_theRenderer->CreateTextureFromFile(m_m_reticleTexturePath.c_str());

    if (hudElement.ChildElementCount() > 0)
    {
        XmlElement const* element = hudElement.FirstChildElement();
        while (element != nullptr)
        {
            Animation animation = Animation(*element);
            m_animations.push_back(animation);
            element = element->NextSiblingElement();
        }
    }
    printf("Hud::Hud    Create Hud with base texture: %s\n", m_baseTexturePath.c_str());
}

Hud::~Hud()
{
    POINTER_SAFE_DELETE(m_shader)
}

Animation* Hud::GetAnimationByName(std::string const& animationName)
{
    for (Animation& animation : m_animations)
    {
        if (animation.m_name == animationName)
        {
            return &animation;
        }
    }
    return nullptr;
}

std::vector<Animation>& Hud::GetAnimations()
{
    return m_animations;
}
