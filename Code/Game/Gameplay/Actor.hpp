#pragma once
#include <vector>

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/ZCylinder.hpp"


class AABB3;
class AABB2;
class Texture;

class Actor
{
public:
    Actor();
    Actor(const Vec3& position, const EulerAngles& orientation, const Rgba8& color, float physicalHeight = 2.0f, float physicalRadius = 1.0f, bool bIsStatic = false);
    virtual ~Actor();

public:
    Vec3        m_position;
    EulerAngles m_orientation;
    Rgba8       m_color;
    bool        m_bIsStatic;
    float       m_physicalHeight;
    float       m_physicalRadius;

private:
    ZCylinder               m_collisionZCylinder;
    std::vector<Vertex_PCU> m_vertexes;
    std::vector<Vertex_PCU> m_vertexesWireframe;
    Texture*                m_texture = nullptr;

public:
    void       Update(float deltaSeconds);
    void       SetPosition(Vec3 newPos);
    ZCylinder& GetColliderZCylinder();
    /// Handle Actor collied with other actors
    /// @param other 
    void OnColliedEnter(Actor* other);
    /// Handle Actor collied with Tile bound in XY plane
    /// @param tileXYBound 
    void OnColliedEnter(AABB2& tileXYBound);
    /// Handle Actor collied with Tile bound in Z axis
    /// @param tileXYZBound 
    void  OnColliedEnter(AABB3& tileXYZBound);
    void  Render() const;
    Mat44 GetModelToWorldTransform() const;

private:
    void InitLocalVertex();
};
