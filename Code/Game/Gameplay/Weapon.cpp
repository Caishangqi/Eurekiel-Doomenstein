#include "Weapon.hpp"

#include "Actor.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRenderSystem.h"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Definition/ActorDefinition.hpp"
#include "Game/Definition/WeaponDefinition.hpp"
#include "Game/Framework/Controller.hpp"

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
        m_owner->m_controller->m_state = "Attack";
        m_lastFireTime                 = m_currentFireTime;
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
            if (!m_owner || !m_owner->m_map) continue;

            // Forward vector from the owner's orientation
            Vec3 fwd, left, up;
            m_owner->m_orientation.GetAsVectors_IFwd_JLeft_KUp(fwd, left, up);

            Vec2  ownerPos2D = Vec2(m_owner->m_position.x, m_owner->m_position.y);
            Vec2  forward2D  = Vec2(fwd.x, fwd.y);
            float halfArc    = m_definition->m_meleeArc * 0.5f;

            Actor* bestTarget   = nullptr;
            float  bestDistSq   = FLT_MAX;
            float  meleeRangeSq = m_definition->m_meleeRange * m_definition->m_meleeRange;

            for (Actor* testActor : m_owner->m_map->m_actors)
            {
                if (!testActor || testActor == m_owner) continue;
                if (testActor->m_bIsDead)continue;
                if (testActor->m_definition->m_faction == m_owner->m_definition->m_faction)continue;
                if (testActor->m_definition->m_faction == "NEUTRAL" || m_owner->m_definition->m_faction == "NEUTRAL") continue;

                Vec2  testPos2D = Vec2(testActor->m_position.x, testActor->m_position.y);
                float distSq    = GetDistanceSquared2D(ownerPos2D, testPos2D);
                if (distSq > meleeRangeSq) continue;
                Vec2  toTarget2D = (testPos2D - ownerPos2D).GetNormalized();
                float angle      = GetAngleDegreesBetweenVectors2D(forward2D, toTarget2D);
                if (angle > halfArc)continue;

                if (distSq < bestDistSq)
                {
                    bestDistSq = distSq;
                    bestTarget = testActor;
                }
            }
            if (bestTarget)
            {
                float damage = g_rng->RollRandomFloatInRange(m_definition->m_meleeDamage.m_min, m_definition->m_meleeDamage.m_max);
                bestTarget->Damage(damage, m_owner->m_handle);
                bestTarget->AddImpulse(m_definition->m_meleeImpulse * fwd);
                printf("Weapon::Fire    Melee: Damaged actor %s\n", bestTarget->m_definition->m_name.c_str());
            }
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

void Weapon::Update(float deltaSeconds)
{
    UpdateAnimation(deltaSeconds);
}

void Weapon::UpdateAnimation(float deltaSeconds)
{
    UNUSED(deltaSeconds)
    if (!m_currentPlayingAnimation)
        return;
    if (m_animationTimer->GetElapsedTime() > m_currentPlayingAnimation->GetAnimationLength())
    {
        m_currentPlayingAnimation = nullptr;
        m_animationTimer->Stop();
    }
}

Animation* Weapon::PlayAnimationByName(std::string animationName, bool force)
{
    Animation* weaponAnim = m_definition->m_hud->GetAnimationByName(animationName);
    if (weaponAnim)
    {
        if (weaponAnim == m_currentPlayingAnimation)
        {
            return weaponAnim;
        }
        else
        {
            /// We want to replace to new animation, force update it whether or not it finished
            if (force)
            {
                m_currentPlayingAnimation = weaponAnim;
                m_animationTimer->Start();
                return weaponAnim;
            }
            else
            {
                if (m_currentPlayingAnimation)
                {
                    bool isCurrentAnimFinished = m_animationTimer->GetElapsedTime() >= m_currentPlayingAnimation->GetAnimationLength();
                    if (isCurrentAnimFinished)
                    {
                        m_currentPlayingAnimation = weaponAnim;
                        m_animationTimer->Start();
                        return weaponAnim;
                    }
                }
                else
                {
                    m_currentPlayingAnimation = weaponAnim;
                    m_animationTimer->Start();
                    return weaponAnim;
                }
            }
        }
    }
    return nullptr;
}

void Weapon::Render() const
{
    Animation* animation = m_currentPlayingAnimation;
    if (animation == nullptr && (int)m_definition->m_hud->GetAnimations().size() > 0) // We use the index 0 animation group
    {
        animation = &m_definition->m_hud->GetAnimations()[0];
    }
}
