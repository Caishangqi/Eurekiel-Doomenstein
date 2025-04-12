#pragma once
#include <map>
#include <string>

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

class AnimationGroup
{
public:
    AnimationGroup(XmlElement const& animationGroupElement, const SpriteSheet& spriteSheet);

    /// Getter
    SpriteAnimDefinition const& GetSpriteAnimation(Vec3 direction);

    std::string                          m_name            = "Default";
    float                                m_scaleBySpeed    = true;
    float                                m_secondsPerFrame = 0.25f;
    SpriteAnimPlaybackType               m_playbackType    = SpriteAnimPlaybackType::LOOP;
    const SpriteSheet&                   m_spriteSheet;
    std::map<Vec3, SpriteAnimDefinition> m_animations;
};
