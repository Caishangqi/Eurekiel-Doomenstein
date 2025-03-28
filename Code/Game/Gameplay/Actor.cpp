#include "Actor.hpp"

#include <cstdio>

#include "Weapon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
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
    m_physicalRadius     = definition->m_physicsRadius;
    m_position           = spawnInfo.m_position;
    m_orientation        = EulerAngles(spawnInfo.m_orientation);
    m_collisionZCylinder = ZCylinder(spawnInfo.m_position, m_physicalRadius, m_physicalHeight, true);
    for (std::string items : definition->m_inventory)
    {
        const WeaponDefinition* weapon = WeaponDefinition::GetByName(items);
        if (weapon)
            m_weapons.push_back(new Weapon(weapon));
    }
    /// AI Controller
    if (m_definition->m_aiEnabled)
    {
        m_aiController = new AIController(m_map);
        m_aiController->Possess(m_handle);
    }
    m_currentWeapon = m_weapons.empty() ? nullptr : m_weapons[0];

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
    printf("Object::Actor    - Destroy Actor and free resources\n");
}


void Actor::Update(float deltaSeconds)
{
    UNUSED(deltaSeconds)
    /// Shitty code :D
    m_collisionZCylinder.m_center.x = m_position.x;
    m_collisionZCylinder.m_center.y = m_position.y;
    m_collisionZCylinder.m_center.z = m_position.z + m_physicalHeight / 2.0f;
    if (m_definition->m_simulated)
    {
        UpdatePhysics(deltaSeconds);
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
}

void Actor::MoveInDirection(Vec3 direction, float speed)
{
    // Normalize the direction to unit vector
    Vec3 dirNormalized = direction.GetNormalized();
    // achieve 'speed' as velocity while overcoming drag
    float dragValue = m_definition->m_drag;
    Vec3  force     = dirNormalized * (speed * dragValue);

    AddForce(force);
}

void Actor::TurnInDirection(Vec3 direction)
{
    if (m_controller && dynamic_cast<PlayerController*>(m_controller))
    {
        m_orientation = EulerAngles(direction);
    }
}

ZCylinder& Actor::GetColliderZCylinder()
{
    return m_collisionZCylinder;
}


void Actor::OnColliedEnter(Actor* other)
{
    if (!other->m_bIsStatic)
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
        Vec2 pos2D = Vec2(m_position.x, m_position.y);
        PushDiscOutOfAABB2D(pos2D, m_physicalRadius, tileXYBound);
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

void Actor::Damage(float damage)
{
}

void Actor::Render() const
{
    if (m_controller && dynamic_cast<PlayerController*>(m_controller))
    {
        PlayerController* playerController = dynamic_cast<PlayerController*>(m_controller);
        if (!playerController->m_bCameraMode)
            return;
    }
    g_theRenderer->SetModelConstants(GetModelToWorldTransform(), m_color);
    g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
    g_theRenderer->BindTexture(m_texture);
    g_theRenderer->DrawVertexArray(m_vertexes);

    g_theRenderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_BACK);
    g_theRenderer->DrawVertexArray(m_vertexesWireframe);
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
    if (m_aiController)
    {
        m_aiController->Possess(m_handle);
    }
}

void Actor::InitLocalVertex()
{
    if (m_physicalHeight <= 0.001f || m_physicalRadius <= 0.001f)
    {
        return;
    }
    // TODO: consider give a better way to handle base center ZCylinder
    ZCylinder localCylinder = m_collisionZCylinder;
    localCylinder.m_center  = Vec3(0, 0, m_physicalHeight / 2.0f);
    AddVertsForCylinderZ3D(m_vertexes, localCylinder, m_color * 0.5f, AABB2::ZERO_TO_ONE);
    AddVertsForCylinderZ3D(m_vertexesWireframe, localCylinder, m_color, AABB2::ZERO_TO_ONE);
    Vec3 localConeStartPoint = Vec3(m_definition->m_physicsRadius - 0.05f, 0, m_definition->m_eyeHeight * 0.85f);
    Vec3 localConeEndPoint   = localConeStartPoint + Vec3(m_definition->m_physicsRadius / 2.f, 0, 0);
    AddVertsForCone3D(m_vertexes, localConeEndPoint, localConeStartPoint, m_definition->m_physicsHeight / 5.0f, m_color * 0.5f);
    AddVertsForCone3D(m_vertexesWireframe, localConeEndPoint, localConeStartPoint, m_definition->m_physicsHeight / 5.0f, m_color);
}
