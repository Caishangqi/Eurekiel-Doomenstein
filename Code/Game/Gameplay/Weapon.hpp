#pragma once
#include "Engine/Math/Vec3.hpp"

class WeaponDefinition;

class Weapon
{
public:
    Weapon() = delete;
    Weapon(const WeaponDefinition* definition);
    ~Weapon();

public:
    /// Checks if the weapon is ready to fire. If so, fires each of the ray casts, projectiles,
    /// and melee attacks defined in the definition. Needs to pass along its owning actor to be
    /// ignored in all raycast and collision checks.
    void Fire();
    /// This, and other utility methods, will be helpful for randomizing weapons with a cone.
    Vec3 GetRandomDirectionInCone();

private:
    WeaponDefinition* m_definition = nullptr;
};
