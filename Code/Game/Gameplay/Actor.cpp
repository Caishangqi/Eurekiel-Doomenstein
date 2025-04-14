#include "Actor.hpp"

#include <cstdio>

#include "Weapon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Definition/ActorDefinition.hpp"
#include "Game/Definition/WeaponDefinition.hpp"
#include "Game/Framework/AIController.hpp"
#include "Game/Framework/PlayerController.hpp"

Actor::Actor(): m_position(Vec3(0, 0, 0)), m_orientation(EulerAngles(0, 0, 0)), m_color(Rgba8::WHITE), m_bIsStatic(false), m_physicalHeight(2.0f), m_physicalRadius(1.0f)
{
    m_collisionZCylinder = ZCylinder(m_position, m_physicalRadius, m_physicalHeight, true);
    InitLocalVertex();
    printf("Object::Actor    + Creating Actor at (%f, %f, %f)\n", m_position.x, m_position.y, m_position.z);
}

Actor::Actor(const Vec3& position, const EulerAngles& orientation, const Rgba8& color, float physicalHeight, float physicalRadius, bool bIsStatic): m_position(position), m_orientation(orientation),
    m_color(color), m_bIsStatic(bIsStatic), m_physicalHeight(physicalHeight), m_physicalRadius(physicalRadius)
{
    m_collisionZCylinder = ZCylinder(m_position, m_physicalRadius, m_physicalHeight, true);
    InitLocalVertex();
    printf("Object::Actor    + Creating Actor at (%f, %f, %f)\n", m_position.x, m_position.y, m_position.z);
}

Actor::Actor(const SpawnInfo& spawnInfo)
{
    ActorDefinition* definition = ActorDefinition::GetByName(spawnInfo.m_actorName);
    if (definition == nullptr)
    {
        ERROR_AND_DIE(Stringf("Actor::Actor    - Actor definition not found for name \"%s\".\n", spawnInfo.m_actorName.c_str()));
    }
    m_definition         = definition;
    m_physicalHeight     = definition->m_physicsHeight;
    m_health             = definition->m_health;
    m_physicalRadius     = definition->m_physicsRadius;
    m_position           = spawnInfo.m_position;
    m_velocity           = spawnInfo.m_velocity;
    m_orientation        = EulerAngles(spawnInfo.m_orientation);
    m_collisionZCylinder = ZCylinder(spawnInfo.m_position, m_physicalRadius, m_physicalHeight, true);
    for (std::string items : definition->m_inventory)
    {
        const WeaponDefinition* weapon = WeaponDefinition::GetByName(items);
        if (weapon)
            m_weapons.push_back(new Weapon(weapon, this));
    }

    /// Color
    if (m_definition->m_name == "Marine")
    {
        m_color = Rgba8::GREEN;
    }
    if (m_definition->m_name == "Demon")
    {
        m_color = Rgba8::RED;
    }

    if (m_definition->m_visible)
        InitLocalVertex();
    printf("Object::Actor    + Creating Actor at (%f, %f, %f)\n", m_position.x, m_position.y, m_position.z);
}


Actor::~Actor()
{
    for (Weapon* weapon : m_weapons)
    {
        POINTER_SAFE_DELETE(weapon)
        POINTER_SAFE_DELETE(m_aiController)
    }
    printf("Object::Actor    - Destroy Actor and free resources\n");
}


void Actor::PostInitialize()
{
    /// AI Controller
    if (m_definition->m_aiEnabled)
    {
        m_aiController = new AIController(m_map);
        m_controller   = m_aiController;
        m_controller->Possess(m_handle);
    }
    m_currentWeapon  = m_weapons.empty() ? nullptr : m_weapons[0];
    m_animationTimer = new Timer(0, g_theGame->m_clock); // Create timer
    m_animationTimer->Start();
}

void Actor::Update(float deltaSeconds)
{
    UNUSED(deltaSeconds)
    if (m_bIsDead)
        m_dead += deltaSeconds;
    if (m_dead > m_definition->m_corpseLifetime)
        m_bIsGarbage = true;

    /// Shitty code :D
    m_collisionZCylinder.m_center.x = m_position.x;
    m_collisionZCylinder.m_center.y = m_position.y;
    m_collisionZCylinder.m_center.z = m_position.z + m_physicalHeight / 2.0f;

    if (m_definition->m_simulated && m_dead == 0.f)
    {
        UpdatePhysics(deltaSeconds);
        if (m_aiController)
            m_aiController->Update(deltaSeconds);
    }
}

void Actor::UpdatePhysics(float deltaSeconds)
{
    float dragValue = m_definition->m_drag;
    Vec3  dragForce = -m_velocity * dragValue;
    AddForce(dragForce);

    m_velocity += m_acceleration * deltaSeconds;
    m_position += m_velocity * deltaSeconds;

    if (!m_definition->m_flying)
    {
        m_position.z = 0.f;
    }
    m_acceleration = Vec3::ZERO;
}

void Actor::AddForce(Vec3 force)
{
    m_acceleration += force;
}

void Actor::AddImpulse(Vec3 impulse)
{
    m_velocity += impulse;
}

void Actor::MoveInDirection(Vec3 direction, float speed)
{
    Vec3 dirNormalized = direction.GetNormalized();
    // 'speed' as velocity while overcoming drag
    float dragValue = m_definition->m_drag;
    Vec3  force     = dirNormalized * (speed * dragValue);
    AddForce(force);
}

void Actor::TurnInDirection(Vec3 direction)
{
    m_orientation = EulerAngles(direction);
}

ZCylinder& Actor::GetColliderZCylinder()
{
    return m_collisionZCylinder;
}


void Actor::OnColliedEnter(Actor* other)
{
    /// Handle projectile
    if (m_bIsDead || other->m_bIsDead)
    {
        return;
    }
    // if both are projectile
    if (other->m_owner && m_owner)
        return;
    // if self are projectile other is not
    if (m_owner && !other->m_owner)
    {
        if (m_owner == other)
        {
            return;
        }
        else
        {
            float randomDamage = g_rng->RollRandomFloatInRange(m_definition->m_damageOnCollide.m_min, m_definition->m_damageOnCollide.m_max);
            other->Damage(randomDamage, m_owner->m_handle);
            Vec3 forward, left, right;
            m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, right);
            other->AddImpulse(m_definition->m_impulseOnCollied * forward);
            SetActorDead();
        }
        return;
    }
    // if self not projectile other is
    if (!m_owner && other->m_owner)
    {
        if (this == other->m_owner)
        {
            return;
        }
        else
        {
            float randomDamage = g_rng->RollRandomFloatInRange(other->m_definition->m_damageOnCollide.m_min, other->m_definition->m_damageOnCollide.m_max);
            Damage(randomDamage, other->m_handle);
            Vec3 forward, left, right;
            other->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, right);
            AddImpulse(other->m_definition->m_impulseOnCollied * forward);
            other->SetActorDead();
        }
        return;
    }

    ///
    if (other->m_definition->m_collidesWithActors)
    {
        float A_bottom = m_position.z;
        float A_top    = m_position.z + m_physicalHeight;
        float B_bottom = other->m_position.z;
        float B_top    = other->m_position.z + other->m_physicalHeight;

        FloatRange rangeA = FloatRange(A_bottom, A_top);
        FloatRange rangeB = FloatRange(B_bottom, B_top);
        if (rangeA.IsOverlappingWith(rangeB))
        {
            Vec2 pos2D      = Vec2(m_position.x, m_position.y);
            Vec2 otherPos2D = Vec2(other->m_position.x, other->m_position.y);
            PushDiscOutOfDisc2D(otherPos2D, other->m_physicalRadius, pos2D, m_physicalRadius);
            other->m_position = Vec3(otherPos2D.x, otherPos2D.y, other->m_position.z);
        }
        else
        {
            // Calculate penetration depths
            float overlapDown = A_top - B_bottom; // How much B intrudes from below
            float overlapUp   = B_top - A_bottom; // How much B intrudes from above

            // Choose minimal displacement direction:
            if (overlapDown < overlapUp)
            {
                // Push B downward so its bottom touches A_top
                other->m_position.z = A_top;
            }
            else
            {
                // Push B upward so its top touches A_bottom
                other->m_position.z = A_bottom - other->m_physicalHeight;
            }
        }
    }
}

void Actor::OnColliedEnter(AABB2& tileXYBound)
{
    if (!m_bIsStatic)
    {
        Vec2 pos2D         = Vec2(m_position.x, m_position.y);
        bool colliedWithXY = PushDiscOutOfAABB2D(pos2D, m_physicalRadius, tileXYBound);
        if (colliedWithXY && m_definition->m_dieOnCollide)
            m_bIsDead = true;
        m_position = Vec3(pos2D.x, pos2D.y, m_position.z);
    }
}

void Actor::OnColliedEnter(AABB3& tileXYZBound)
{
    if (!m_bIsStatic)
    {
        float zCylinderMaxZ, zCylinderMinZ;
        zCylinderMaxZ = m_position.z + m_physicalHeight;
        zCylinderMinZ = m_position.z;
        if (zCylinderMaxZ > tileXYZBound.m_maxs.z || zCylinderMinZ < tileXYZBound.m_mins.z)
        {
            if (m_definition->m_dieOnCollide)
                m_bIsDead = true;
        }
        if (zCylinderMaxZ > tileXYZBound.m_maxs.z)
        {
            zCylinderMaxZ = tileXYZBound.m_maxs.z;
            m_position.z  = zCylinderMaxZ - m_physicalHeight;
        }
        if (zCylinderMinZ < tileXYZBound.m_mins.z)
        {
            zCylinderMinZ = tileXYZBound.m_mins.z;
            m_position.z  = zCylinderMinZ;
        }
    }
}

void Actor::Damage(float damage, ActorHandle instigator)
{
    m_health -= damage;
    printf("Actor::Damage    Actor %s was Damaged, health now %f\n", m_definition->m_name.c_str(), m_health);
    if (m_health <= 0.f)
    {
        SetActorDead();
    }
    if (m_aiController)
        m_aiController->DamagedBy(instigator);
}

bool Actor::SetActorDead(bool bNewDead)
{
    m_bIsDead = bNewDead;
    for (Vertex_PCU& m_vertex : m_vertexes)
    {
        m_vertex.m_color = m_vertex.m_color * 0.4f;
    }
    for (Vertex_PCU& m_vertexes_wireframe : m_vertexesWireframe)
    {
        m_vertexes_wireframe.m_color = m_vertexes_wireframe.m_color * 0.4f;
    }
    for (Vertex_PCU& m_vertexes_cone : m_vertexesCone)
    {
        m_vertexes_cone.m_color = m_vertexes_cone.m_color * 0.4f;
    }
    for (Vertex_PCU& m_vertexes_cone_wireframe : m_vertexesConeWireframe)
    {
        m_vertexes_cone_wireframe.m_color = m_vertexes_cone_wireframe.m_color * 0.4f;
    }
    return m_bIsDead;
}

void Actor::EquipWeapon(unsigned int index)
{
    if (index < m_weapons.size())
    {
        if (m_currentWeapon != m_weapons[index])
        {
            m_currentWeapon = m_weapons[index];
            printf("Actor::SwitchInventory    Weapon switched to %s\n", m_currentWeapon->m_definition->m_name.c_str());
        }
    }
}

void Actor::Attack()
{
    if (m_currentWeapon)
        m_currentWeapon->Fire();
}

Vec3 Actor::GetActorEyePosition()
{
    return m_position + Vec3(0, 0, m_definition->m_eyeHeight);
}

void Actor::Render() const
{
    if (!m_definition->m_visible) return; // Check if visible. If not, return.
    if (m_controller && dynamic_cast<PlayerController*>(m_controller)) // Check if we are the rendering player and not in free fly mode. If so, return.
    {
        PlayerController* playerController = dynamic_cast<PlayerController*>(m_controller);
        if (!playerController->m_bCameraMode)
            return;
    }
    Mat44 localToWorldMat;
    if (m_definition->m_billboardType == "None")
        localToWorldMat = GetModelToWorldTransform();
    else
    {
        if (m_definition->m_billboardType == "WorldUpFacing")
        {
            Mat44 cameraTransform = Mat44::MakeTranslation3D(g_theGame->m_player->m_position);
            cameraTransform.Append(g_theGame->m_player->m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
            localToWorldMat = Mat44::MakeTranslation3D(m_position);
            localToWorldMat.Append(GetBillboardTransform(BillboardType::WORLD_UP_FACING, cameraTransform, m_position));
        }
        else
        {
            localToWorldMat = GetModelToWorldTransform();
        }
    }

    /// Get facing sprite UVs.
    Vec3 dirCameraToActor = (m_position - g_theGame->m_player->m_position).GetXY().GetNormalized().GetAsVec3();
    Vec3 viewingDirection = GetModelToWorldTransform().GetOrthonormalInverse().TransformVectorQuantity3D(dirCameraToActor);

    AnimationGroup* animationGroup = m_currentPlayingAnimationGroup;
    if (animationGroup == nullptr && (int)m_definition->m_animationGroups.size() > 0) // We use the index 0 animation group
    {
        animationGroup = &m_definition->m_animationGroups[0];
    }

    const SpriteAnimDefinition* anim         = &animationGroup->GetSpriteAnimation(viewingDirection);
    const SpriteDefinition      spriteAtTime = anim->GetSpriteDefAtTime(m_animationTimer->GetElapsedTime());
    AABB2                       uvAtTime     = spriteAtTime.GetUVs();


    Vec2 spriteOffSet = -m_definition->m_size * m_definition->m_pivot;
    Vec3 bottomLeft   = Vec3(0, spriteOffSet.x, spriteOffSet.y);
    Vec3 bottomRight  = bottomLeft + Vec3(0, m_definition->m_size.x, 0);
    Vec3 topLeft      = bottomLeft + Vec3(0, 0, m_definition->m_size.y);
    Vec3 topRight     = bottomRight + Vec3(0, 0, m_definition->m_size.y);

    /// Create geometry.
    bool                       bIsLit = m_definition->m_renderLit;
    std::vector<Vertex_PCUTBN> vertexesLit;
    std::vector<Vertex_PCU>    vertexesUnlit;
    if (bIsLit)
    {
        if (m_definition->m_renderRounded)
        {
            vertexesLit.reserve(8192);
            AddVertsForRoundedQuad3D(vertexesLit, bottomLeft, bottomRight, topRight, topLeft, Rgba8::WHITE, uvAtTime);
        }
        else
        {
            vertexesUnlit.reserve(8192);
            AddVertsForQuad3D(vertexesUnlit, bottomLeft, bottomRight, topRight, topLeft, Rgba8::WHITE, uvAtTime);
        }
        g_theRenderer->SetModelConstants(localToWorldMat, Rgba8::WHITE);
        g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
        g_theRenderer->SetLightConstants(m_map->m_sunDirection, m_map->m_sunIntensity, m_map->m_ambientIntensity);
        g_theRenderer->BindTexture(&spriteAtTime.GetTexture());
        g_theRenderer->DrawVertexArray(vertexesLit);
        return;
    }

    g_theRenderer->SetModelConstants(localToWorldMat, m_color);
    g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
    g_theRenderer->BindTexture(m_texture);
    g_theRenderer->DrawVertexArray(m_vertexes);
    if (!m_owner)
        g_theRenderer->DrawVertexArray(m_vertexesCone);

    g_theRenderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_BACK);
    g_theRenderer->DrawVertexArray(m_vertexesWireframe);
    if (!m_owner)
        g_theRenderer->DrawVertexArray(m_vertexesConeWireframe);
    g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
}

Mat44 Actor::GetModelToWorldTransform() const
{
    Mat44       matTranslation = Mat44::MakeTranslation3D(m_position);
    EulerAngles orientationRender;
    orientationRender.m_yawDegrees = m_orientation.m_yawDegrees;
    matTranslation.Append(orientationRender.GetAsMatrix_IFwd_JLeft_KUp());
    return matTranslation;
}

void Actor::OnPossessed(Controller* controller)
{
    m_controller = controller;
}

void Actor::OnUnpossessed()
{
    m_controller = nullptr;
    if (m_aiController)
    {
        m_controller = m_aiController;
    }
}

AnimationGroup* Actor::PlayAnimationByName(std::string& animationName)
{
    AnimationGroup* foundedGroup = m_definition->GetAnimationGroupByName(animationName);
    if (foundedGroup)
    {
        if (foundedGroup == m_currentPlayingAnimationGroup)
        {
            return foundedGroup;
        }
        else
        {
            m_currentPlayingAnimationGroup = foundedGroup;
            return foundedGroup;
        }
    }
    return nullptr;
}

void Actor::InitLocalVertex()
{
    if (m_physicalHeight <= 0.001f || m_physicalRadius <= 0.001f)
    {
        return;
    }
    m_vertexes.reserve(8192);
    // TODO: consider give a better way to handle base center ZCylinder
    ZCylinder localCylinder = m_collisionZCylinder;
    localCylinder.m_center  = Vec3(0, 0, m_physicalHeight / 2.0f);
    AddVertsForCylinderZ3D(m_vertexes, localCylinder, m_color * 0.5f, AABB2::ZERO_TO_ONE);
    AddVertsForCylinderZ3D(m_vertexesWireframe, localCylinder, m_color, AABB2::ZERO_TO_ONE);
    Vec3 localConeStartPoint = Vec3(m_definition->m_physicsRadius - 0.05f, 0, m_definition->m_eyeHeight * 0.85f);
    Vec3 localConeEndPoint   = localConeStartPoint + Vec3(m_definition->m_physicsRadius / 2.f, 0, 0);
    AddVertsForCone3D(m_vertexesCone, localConeEndPoint, localConeStartPoint, m_definition->m_physicsHeight / 5.0f, m_color * 0.5f);
    AddVertsForCone3D(m_vertexesConeWireframe, localConeEndPoint, localConeStartPoint, m_definition->m_physicsHeight / 5.0f, m_color);
}
