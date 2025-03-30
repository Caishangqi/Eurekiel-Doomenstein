#include "Weapon.hpp"

#include "Actor.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRenderSystem.h"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Definition/ActorDefinition.hpp"
#include "Game/Definition/WeaponDefinition.hpp"

Weapon::Weapon(const WeaponDefinition* definition, Actor* owner): m_definition(definition), m_owner(owner)
{
}

Weapon::~Weapon()
{
    m_owner = nullptr;
}

void Weapon::Fire()
{
    int rayCount        = m_definition->m_rayCount;
    int projectileCount = m_definition->m_projectileCount;
    int meleeCount      = m_definition->m_meleeCount;

    float m_currentFireTime   = g_theGame->m_clock->GetTotalSeconds();
    float m_timeSinceLastFire = m_currentFireTime - m_lastFireTime;
    if (m_timeSinceLastFire > m_definition->m_refireTime)
    {
        printf("Weapon::Fire    Weapon fired by %s\n", m_owner->m_definition->m_name.c_str());
        m_lastFireTime = m_currentFireTime;
        /// Fire logic here
        while (rayCount > 0)
        {
            ActorHandle hitActorHandle;
            EulerAngles randomDirection = GetRandomDirectionInCone(m_owner->m_orientation, m_definition->m_rayCone);
            Vec3        forward, left, up;
            float       rayRange        = m_definition->m_rayRange;
            Vec3        startPos        = m_owner->m_position + Vec3(0, 0, m_owner->m_definition->m_eyeHeight);
            Vec3        startPosGraphic = startPos - Vec3(0, 0, 0.2f);
            randomDirection.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
            RaycastResult3D raycastResult = m_owner->m_map->RaycastAll(m_owner, hitActorHandle, startPos, forward, m_definition->m_rayRange);
            if (raycastResult.m_didImpact)
            {
                DebugAddWorldCylinder(startPosGraphic, raycastResult.m_impactPos, 0.01f, 10.f, Rgba8::YELLOW, Rgba8::YELLOW, DebugRenderMode::X_RAY);
            }
            else
            {
                DebugAddWorldCylinder(startPosGraphic, raycastResult.m_rayStartPos + raycastResult.m_rayFwdNormal * rayRange, 0.01f, 10.f, Rgba8::YELLOW, Rgba8::YELLOW, DebugRenderMode::X_RAY);
            }
            Actor* hitActor = m_owner->m_map->GetActorByHandle(hitActorHandle);
            if (hitActor && hitActor->m_handle.IsValid())
            {
                float damage = g_rng->RollRandomFloatInRange(m_definition->m_rayDamage.m_min, m_definition->m_rayDamage.m_max);
                hitActor->Damage(damage, m_owner->m_handle);
                hitActor->AddImpulse(m_definition->m_rayImpulse * forward);
            }
            rayCount--;
        }
        while (projectileCount > 0)
        {
            Vec3 startPos = m_owner->m_position + Vec3(0, 0, m_owner->m_definition->m_eyeHeight);
            Vec3 forward, left, up;

            EulerAngles randomDirection = GetRandomDirectionInCone(m_owner->m_orientation, m_definition->m_projectileCone);
            randomDirection.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
            Vec3      projectileSpeed = forward * m_definition->m_projectileSpeed;
            SpawnInfo spawnInfo{m_definition->m_projectileActor, m_owner->m_definition->m_faction, startPos, Vec3(randomDirection), projectileSpeed};
            Actor*    projectile = m_owner->m_map->SpawnActor(spawnInfo);
            projectile->m_owner  = m_owner;
            projectileCount--;
        }
        while (meleeCount > 0)
        {
            meleeCount--;
        }
    }
}

/// TODO: use native Vec3 internal direction methods to get random direction in a cone
Vec3 Weapon::GetRandomDirectionInCone(Vec3 weaponOrientation, float degreeOfVariation)
{
    float       variation   = g_rng->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    EulerAngles orientation = EulerAngles(weaponOrientation.x + variation, weaponOrientation.y + variation, weaponOrientation.z + variation);
    return Vec3(orientation);
}

EulerAngles Weapon::GetRandomDirectionInCone(EulerAngles weaponOrientation, float degreeOfVariation)
{
    float       variationYaw   = g_rng->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    float       variationPitch = g_rng->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    float       variationRow   = g_rng->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    EulerAngles newDirection   = EulerAngles(weaponOrientation.m_yawDegrees + variationYaw, weaponOrientation.m_pitchDegrees + variationPitch, weaponOrientation.m_rollDegrees + variationRow);
    return newDirection;
}
