#include "Weapon.hpp"

#include "Actor.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRenderSystem.h"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Definition/ActorDefinition.hpp"
#include "Game/Definition/WeaponDefinition.hpp"
#include "Game/Framework/Controller.hpp"
#include "Game/Framework/PlayerController.hpp"
#include "Save/PlayerSaveSubsystem.hpp"

Weapon::Weapon(WeaponDefinition* definition, Actor* owner): m_owner(owner), m_definition(definition)
{
    m_animationTimer = new Timer(0, g_theGame->m_clock);

    /// Init hud base bound
    if (m_definition->m_hud != nullptr)
    {
        Texture* baseTexture   = m_definition->m_hud->m_baseTexture;
        IntVec2  baseDimension = baseTexture->GetDimensions();
        float    multiplier    = g_theGame->m_screenSpace.m_maxs.x / static_cast<float>(baseDimension.x);
        m_hudBaseBound         = AABB2(Vec2(0.0f, 0.0f), Vec2(g_theGame->m_screenSpace.m_maxs.x, static_cast<float>(baseDimension.y) * multiplier));
    }
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
        SoundID weaponFireSound        = m_definition->GetSoundByName("Fire")->GetSoundID();
        g_theAudio->StartSoundAt(weaponFireSound, m_owner->m_position);
        if (m_definition->m_hud)
        {
            PlayAnimationByName("Attack");
        }
        /// Handle player fire animation logic
        auto player = dynamic_cast<PlayerController*>(m_owner->m_controller);
        if (player)
        {
            player->GetActor()->PlayAnimationByName("Attack");
        }
        /// End of Handle player fire animation logic.
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
            Actor*          hitActor      = m_owner->m_map->GetActorByHandle(hitActorHandle);
            if (raycastResult.m_didImpact)
            {
                if (!hitActor)
                {
                    SpawnInfo particleSpawnInfo;
                    particleSpawnInfo.m_position  = raycastResult.m_impactPos;
                    particleSpawnInfo.m_actorName = "BulletHit";
                    g_theGame->m_map->SpawnActor(particleSpawnInfo);
                }
                if (IS_DEBUG_ENABLED())
                    DebugAddWorldCylinder(startPosGraphic, raycastResult.m_impactPos, 0.01f, 10.f, Rgba8::YELLOW, Rgba8::YELLOW, DebugRenderMode::X_RAY);
            }
            else
            {
                if (IS_DEBUG_ENABLED())
                    DebugAddWorldCylinder(startPosGraphic, raycastResult.m_rayStartPos + raycastResult.m_rayFwdNormal * rayRange, 0.01f, 10.f, Rgba8::YELLOW, Rgba8::YELLOW, DebugRenderMode::X_RAY);
            }
            if (hitActor && hitActor->m_handle.IsValid())
            {
                float damage = g_rng->RollRandomFloatInRange(m_definition->m_rayDamage.m_min, m_definition->m_rayDamage.m_max);
                hitActor->Damage(damage, m_owner->m_handle);
                hitActor->AddImpulse(m_definition->m_rayImpulse * forward);
                SpawnInfo particleSpawnInfo;
                particleSpawnInfo.m_position  = raycastResult.m_impactPos;
                particleSpawnInfo.m_actorName = "BloodSplatter";
                g_theGame->m_map->SpawnActor(particleSpawnInfo);
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

            auto  ownerPos2D = Vec2(m_owner->m_position.x, m_owner->m_position.y);
            auto  forward2D  = Vec2(fwd.x, fwd.y);
            float halfArc    = m_definition->m_meleeArc * 0.5f;

            Actor* bestTarget = nullptr;
            float  bestDist   = FLT_MAX;
            float  meleeRange = m_definition->m_meleeRange;

            for (Actor* testActor : m_owner->m_map->m_actors)
            {
                if (!testActor || testActor == m_owner) continue;
                if (testActor->m_bIsDead)continue;
                if (testActor->m_definition->m_faction == m_owner->m_definition->m_faction)continue;
                if (testActor->m_definition->m_faction == "NEUTRAL" || m_owner->m_definition->m_faction == "NEUTRAL") continue;

                auto  testPos2D         = Vec2(testActor->m_position.x, testActor->m_position.y);
                float dist              = GetDistance2D(ownerPos2D, testPos2D);
                float distExcludeRadius = dist - testActor->m_definition->m_physicsRadius - m_owner->m_physicalRadius;
                if (distExcludeRadius > meleeRange - m_owner->m_physicalRadius) continue;
                Vec2  toTarget2D = (testPos2D - ownerPos2D).GetNormalized();
                float angle      = GetAngleDegreesBetweenVectors2D(forward2D, toTarget2D);
                if (angle > halfArc)continue;

                if (dist < bestDist)
                {
                    bestDist   = dist;
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
    float variation   = g_rng->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    auto  orientation = EulerAngles(weaponOrientation.x + variation, weaponOrientation.y + variation, weaponOrientation.z + variation);
    return Vec3(orientation);
}

EulerAngles Weapon::GetRandomDirectionInCone(EulerAngles weaponOrientation, float degreeOfVariation)
{
    float variationYaw   = g_rng->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    float variationPitch = g_rng->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    float variationRow   = g_rng->RollRandomFloatInRange(-degreeOfVariation, degreeOfVariation);
    auto  newDirection   = EulerAngles(weaponOrientation.m_yawDegrees + variationYaw, weaponOrientation.m_pitchDegrees + variationPitch, weaponOrientation.m_rollDegrees + variationRow);
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
        /// We want to replace to new animation, force update it whether or not it finished
        if (force)
        {
            m_currentPlayingAnimation = weaponAnim;
            m_animationTimer->Start();
            return weaponAnim;
        }
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
    return nullptr;
}

void Weapon::Render() const
{
    g_theRenderer->BindShader(m_definition->m_hud->m_shader);
    g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
    RenderWeaponBase();
    RenderWeaponReticle();
    RenderWeaponAnim();
    RenderWeaponHudText();
}

void Weapon::RenderWeaponBase() const
{
    std::vector<Vertex_PCU> vertexes;
    vertexes.reserve(8192);

    //Vec2 spriteOffSet = -Vec2(m_definition->m_hud->m_spriteSize) * m_definition->m_hud->m_spritePivot;
    /// Hud Base TODO: Handle multiplayer
    Texture* baseTexture = m_definition->m_hud->m_baseTexture;
    AddVertsForAABB2D(vertexes, m_hudBaseBound, Rgba8::WHITE);
    g_theRenderer->BindTexture(baseTexture);
    g_theRenderer->DrawVertexArray(vertexes);
}

void Weapon::RenderWeaponReticle() const
{
    std::vector<Vertex_PCU> vertexes;
    vertexes.reserve(1024);
    /// Reticle
    Texture* reticleTexture   = m_definition->m_hud->m_reticleTexture;
    Vec2     reticleDimension = reticleTexture->GetDimensions();

    Vec2 reticleSpriteOffSet = -reticleDimension * m_definition->m_hud->m_spritePivot;
    auto reticleBound        = AABB2(Vec2(g_theGame->m_screenSpace.m_maxs / 2.0f), (Vec2(g_theGame->m_screenSpace.m_maxs / 2.0f) + Vec2(reticleDimension)));

    /// Change the dimension base on split screen y
    Vec2  dim            = reticleBound.GetDimensions();
    AABB2 screenViewport = m_owner->m_controller->m_screenViewport;
    float multiplier     = g_theGame->m_screenSpace.GetDimensions().y / screenViewport.GetDimensions().y;
    dim.x /= multiplier;
    reticleBound.SetDimensions(dim);
    /// End of Change

    AddVertsForAABB2D(vertexes, reticleBound, Rgba8::WHITE);
    g_theRenderer->BindTexture(reticleTexture);
    g_theRenderer->DrawVertexArray(vertexes);
}

void Weapon::RenderWeaponHudText() const
{
    g_theRenderer->BindTexture(nullptr);
    auto player = dynamic_cast<PlayerController*>(m_owner->m_controller);
    if (!player) return;
    if (!player->GetActor() || player->GetActor()->m_bIsDead)
        return; // Handle player death not render hud text
    std::vector<Vertex_PCU> vertexes;
    vertexes.reserve(1024);
    BitmapFont* g_testFont     = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
    AABB2       boundingBox    = m_hudBaseBound;
    Vec2        dim            = boundingBox.GetDimensions();
    AABB2       screenViewport = m_owner->m_controller->m_screenViewport;
    float       multiplier     = g_theGame->m_screenSpace.GetDimensions().y / screenViewport.GetDimensions().y;
    g_testFont->AddVertsForTextInBox2D(vertexes, Stringf("%d", static_cast<int>(m_owner->m_health)), boundingBox, 40.f, Rgba8::WHITE, 1 / multiplier, Vec2(0.29f, 0.5f));
    g_testFont->AddVertsForTextInBox2D(vertexes, Stringf("%d", PlayerSaveSubsystem::GetPlayerSaveData(player->m_index)->m_numOfKilled), boundingBox, 40.f, Rgba8::WHITE, 1 / multiplier,
                                       Vec2(0.05f, 0.5f));
    g_testFont->AddVertsForTextInBox2D(vertexes, Stringf("%d", PlayerSaveSubsystem::GetPlayerSaveData(player->m_index)->m_numOfDeaths), boundingBox, 40.f, Rgba8::WHITE, 1 / multiplier,
                                       Vec2(0.95f, 0.5f));
    g_theRenderer->BindTexture(&g_testFont->GetTexture());
    g_theRenderer->DrawVertexArray(vertexes);
    g_theRenderer->BindTexture(nullptr);
}

void Weapon::RenderWeaponAnim() const
{
    auto player = dynamic_cast<PlayerController*>(m_owner->m_controller);
    if (!player->GetActor() || player->GetActor()->m_bIsDead)
        return; // Handle player death not render anim
    std::vector<Vertex_PCU> vertexes;
    vertexes.reserve(1024);
    Animation* animation = m_currentPlayingAnimation;
    if (animation == nullptr && static_cast<int>(m_definition->m_hud->GetAnimations().size()) > 0) // We use the index 0 animation group
    {
        animation = &m_definition->m_hud->GetAnimations()[0];
    }
    const SpriteAnimDefinition* anim         = animation->GetAnimationDefinition();
    const SpriteDefinition      spriteAtTime = anim->GetSpriteDefAtTime(m_animationTimer->GetElapsedTime());
    AABB2                       uvAtTime     = spriteAtTime.GetUVs();

    Vec2 spriteOffSet = -Vec2(m_definition->m_hud->m_spriteSize) * m_definition->m_hud->m_spritePivot;

    IntVec2 boundSize = m_definition->m_hud->m_spriteSize;
    auto    bound     = AABB2(Vec2(g_theGame->m_screenSpace.m_maxs.x / 2.0f, 0.f), Vec2(g_theGame->m_screenSpace.m_maxs.x / 2.0f, 0.f) + Vec2(boundSize));

    /// Change the dimension base on split screen y
    Vec2  dim            = bound.GetDimensions();
    AABB2 screenViewport = m_owner->m_controller->m_screenViewport;
    float multiplier     = g_theGame->m_screenSpace.GetDimensions().y / screenViewport.GetDimensions().y;
    dim.x /= multiplier;
    bound.SetDimensions(dim);
    /// End of Change

    bound.m_mins += spriteOffSet;
    bound.m_maxs += spriteOffSet;
    bound.Translate(Vec2(0, m_hudBaseBound.m_maxs.y)); // Shitty hardcode
    AddVertsForAABB2D(vertexes, bound, Rgba8::WHITE, uvAtTime.m_mins, uvAtTime.m_maxs);
    AddVertsForAABB2D(vertexes, uvAtTime, Rgba8::WHITE);
    g_theRenderer->BindTexture(&spriteAtTime.GetTexture());
    g_theRenderer->DrawVertexArray(vertexes);
}
