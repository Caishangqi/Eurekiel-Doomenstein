#pragma once
#include <map>
#include <string>

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

class AnimationGroup
{
public:
    AnimationGroup(const XmlElement& animationGroupElement, const SpriteSheet& spriteSheet);

    /// Getter
    const SpriteAnimDefinition& GetSpriteAnimation(Vec3 direction);
    float                       GetAnimationLength();
    int                         GetAnimationTotalFrame();

    std::string                          m_name            = "Default";
    float                                m_scaleBySpeed    = true;
    float                                m_secondsPerFrame = 0.25f;
    SpriteAnimPlaybackType               m_playbackType    = SpriteAnimPlaybackType::LOOP;
    const SpriteSheet&                   m_spriteSheet;
    std::map<Vec3, SpriteAnimDefinition> m_animations;
};
