#pragma once
#include "Engine/Math/Vec3.hpp"

class Actor;
class WeaponDefinition;

class Weapon
{
    friend class Actor;
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
    Vec3        GetRandomDirectionInCone(Vec3 weaponOrientation, float degreeOfVariation);
    /// 
    /// @param weaponOrientation 
    /// @param degreeOfVariation 
    /// @return 
    EulerAngles GetRandomDirectionInCone(EulerAngles weaponOrientation, float degreeOfVariation);

protected:
    Actor*                  m_owner        = nullptr;
    const WeaponDefinition* m_definition   = nullptr;
    float                   m_lastFireTime = 0.f;
};
