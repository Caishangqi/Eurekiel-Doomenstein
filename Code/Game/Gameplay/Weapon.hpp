#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec3.hpp"

class Timer;
class Animation;
class Actor;
class WeaponDefinition;

class Weapon
{
    friend class Actor;
    friend class Map;
    friend class AIController;

public:
    Weapon() = delete;
    Weapon(const WeaponDefinition* definition, Actor* owner);
    ~Weapon();

public:
    /// Checks if the weapon is ready to fire. If so, fires each of the ray casts, projectiles,
    /// and melee attacks defined in the definition. Needs to pass along its owning actor to be
    /// ignored in all raycast and collision checks.
    void Fire();
    /// This, and other utility methods, will be helpful for randomizing weapons with a cone.
    /// @param weaponOrientation 
    /// @param degreeOfVariation 
    /// @return 
    Vec3 GetRandomDirectionInCone(Vec3 weaponOrientation, float degreeOfVariation);
    /// 
    /// @param weaponOrientation 
    /// @param degreeOfVariation 
    /// @return 
    EulerAngles GetRandomDirectionInCone(EulerAngles weaponOrientation, float degreeOfVariation);

    void Update(float deltaSeconds);
    void UpdateAnimation(float deltaSeconds);

    Animation* PlayAnimationByName(std::string animationName, bool force = false);

    void Render() const;
    void RenderWeaponBase() const;
    void RenderWeaponReticle() const;
    void RenderWeaponHudText() const;
    void RenderWeaponAnim() const;

protected:
    Actor*                  m_owner        = nullptr;
    const WeaponDefinition* m_definition   = nullptr;
    float                   m_lastFireTime = 0.f;

    AABB2 m_hudBaseBound; // we calculate the bound that Seamlessly connect the weapon texture

    Animation* m_currentPlayingAnimation = nullptr;
    Timer*     m_animationTimer          = nullptr;
};
