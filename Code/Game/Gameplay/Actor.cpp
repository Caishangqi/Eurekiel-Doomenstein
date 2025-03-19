#include "Actor.hpp"

#include <cstdio>

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"

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

Actor::~Actor()
{
    printf("Object::Actor    - Destroy Actor and free resources\n");
}

void Actor::Update(float deltaSeconds)
{
    m_collisionZCylinder.m_center.x = m_position.x;
    m_collisionZCylinder.m_center.y = m_position.y;
    m_collisionZCylinder.m_center.z = m_position.z + m_physicalHeight / 2.0f;
}

void Actor::SetPosition(Vec3 newPos)
{
    m_position                      = newPos;
    m_collisionZCylinder.m_center.x = newPos.x;
    m_collisionZCylinder.m_center.y = newPos.y;
    m_collisionZCylinder.m_center.z = newPos.z + m_physicalHeight / 2.0f;
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
            Vec2 pos2D        = Vec2(m_position.x, m_position.y);
            Vec2 otherPos2D   = Vec2(other->m_position.x, other->m_position.y);
            bool pushed       = PushDiscOutOfDisc2D(otherPos2D, other->m_physicalRadius, pos2D, m_physicalRadius);
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

void Actor::Render() const
{
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
    Mat44 matTranslation = Mat44::MakeTranslation3D(m_position);
    matTranslation.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
    return matTranslation;
}

void Actor::InitLocalVertex()
{
    // TODO: consider give a better way to handle base center ZCylinder
    ZCylinder localCylinder = m_collisionZCylinder;
    localCylinder.m_center  = Vec3(0, 0, m_physicalHeight / 2.0f);
    AddVertsForCylinderZ3D(m_vertexes, localCylinder, m_color * 0.5f, AABB2::ZERO_TO_ONE);
    AddVertsForCylinderZ3D(m_vertexesWireframe, localCylinder, m_color, AABB2::ZERO_TO_ONE);
}
