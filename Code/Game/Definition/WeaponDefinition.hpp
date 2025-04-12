#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Game/Framework/Hud.hpp"
#include "Game/Framework/Sound.hpp"


class WeaponDefinition
{
public:
    static std::vector<WeaponDefinition> s_definitions;
    static void                          LoadDefinitions(const char* path);
    static void                          ClearDefinitions();
    static const WeaponDefinition*       GetByName(const std::string& name);

public:
    WeaponDefinition(XmlElement const& mapDefElement);
    Sound* GetSoundByName(std::string const& soundName);

public:
    // Definition name of the weapon to add to this actor when it is spawned.
    std::string m_name = "Default";
    // The time that must elapse between firing, in seconds.
    float m_refireTime = 0.f;
    // The number of rays to cast each time the weapon is fired. Each ray can potentially hit an actor and do damage.
    int m_rayCount = 0;
    // Maximum angle variation for each ray cast, in degrees. Each shot fired should be randomly distributed in a cone of this angle relative forward direction of the firing actor.
    float m_rayCone = 0.f;
    // The distance of each ray cast, in world units. 
    float m_rayRange = 0.f;
    // Minimum and maximum damage expressed as a float range. Each shot fired should do a random amount of damage in this range.
    FloatRange m_rayDamage = FloatRange(0.0f, 0.0f);
    // The amount of impulse to impart to any actor hit by a ray. Impulse should be in the direction of the ray.
    float m_rayImpulse = 0.f;
    // Number of projectiles to launch each time the weapon is fired.
    int m_projectileCount = 0;
    // Maximum angle variation in degrees for each projectile launched cast. Each projectile launched should have its velocity randomly distributed in a cone of this angle relative to the forward direction of the firing actor.
    float m_projectileCone = 0.f;
    // Magnitude of the velocity given to each projectile launched.
    float m_projectileSpeed = 0.f;
    // Definition name for the actor that should be spawned when a projectile is launched.
    std::string m_projectileActor = "";
    // Number of melee attacks that should occur each time the weapon is fired.
    int m_meleeCount = 0;
    // Arc in which melee attacks occur, in degrees.
    float m_meleeArc = 0.f;
    // Range of each melee attack, in world units.
    float m_meleeRange = 0.f;
    // Minimum and maximum damage expressed as a float range. Each melee attack should do a random amount of damage in this range.
    FloatRange m_meleeDamage = FloatRange(0.0f, 0.0f);
    // The amount of impulse to impart to any actor hit by a melee attack. Impulse should be in the forward direction of the firing actor.
    float m_meleeImpulse = 0.f;
    // The Hud of weapon
    Hud* m_hud = nullptr;
    // Sound list of weapon
    std::vector<Sound> m_sounds;
};
