#pragma once
#include <string>
#include <vector>

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/Framework/AnimationGroup.hpp"
#include "Game/Framework/Sound.hpp"

class SpriteSheet;
class Shader;

class ActorDefinition
{
public:
    static std::vector<ActorDefinition> s_definitions;
    static void                         LoadDefinitions(const char* path);
    static void                         ClearDefinitions();
    static ActorDefinition*             GetByName(const std::string& name);

    ActorDefinition(const XmlElement& actorDefElement);
    AnimationGroup* GetAnimationGroupByName(std::string& name);
    Sound*          GetSoundByName(std::string name);

    /// Base
    std::string m_name           = "Default";
    std::string m_faction        = "NEUTRAL";
    float       m_health         = 1.0f;
    bool        m_canBePossessed = false;
    float       m_corpseLifetime = 0.0f;
    bool        m_visible        = false;
    bool        m_dieOnSpawn     = false;
    /// Collision
    float      m_physicsRadius      = 0.0f;
    float      m_physicsHeight      = 0.0f;
    bool       m_collidesWithWorld  = false;
    bool       m_collidesWithActors = false;
    bool       m_dieOnCollide       = false;
    FloatRange m_damageOnCollide;
    float      m_impulseOnCollied = 0.f;
    /// Physics
    bool  m_simulated = true;
    bool  m_flying    = false;
    float m_walkSpeed = 0.f;
    float m_runSpeed  = 0.0f;
    float m_turnSpeed = 0.0f;
    float m_drag      = 0.0f;
    /// Camera
    float m_eyeHeight = 0.f;
    float m_cameraFOV = 60.0f;
    /// AI
    bool  m_aiEnabled   = false;
    float m_sightRadius = 0.0f;
    float m_sightAngle  = 0.0f;
    /// Visual
    Vec2                        m_size;
    Vec2                        m_pivot;
    std::string                 m_billboardType = "None";
    bool                        m_renderLit     = false;
    bool                        m_renderRounded = false;
    Shader*                     m_shader        = nullptr;
    SpriteSheet*                m_spriteSheet   = nullptr;
    IntVec2                     m_cellCount     = IntVec2(8, 9);
    std::vector<AnimationGroup> m_animationGroups;
    std::vector<Sound>          m_sounds;
    /// Sounds

    /// Inventory
    std::vector<std::string> m_inventory = {};
};
