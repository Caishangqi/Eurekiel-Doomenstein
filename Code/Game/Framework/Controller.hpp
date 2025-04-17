#pragma once
#include "ActorHandle.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"

class Camera;
class Actor;
class Map;

class Controller
{
public:
    Controller(Map* map);
    virtual ~Controller();

public:
    ActorHandle  m_actorHandle; // Handle of our currently possessed actor or INVALID if no actor is possessed.
    Map*         m_map   = nullptr; // Reference to the current map for purposes of dereferencing actor handles.
    int          m_index = -1;
    std::string  m_state = "None"; // Track for state
    virtual void Update(float deltaTime);
    /// Unpossess any currently possessed actor and possess a new one. Notify each actor so it can check for
    /// restoring AI controllers or handling other changes of possession logic.
    /// @param actorHandle 
    virtual void Possess(ActorHandle& actorHandle);
    /// Returns the currently possessed actor or null if no actor is possessed.
    virtual Actor* GetActor();

    /// Setter
    void  SetControllerIndex(int index);
    AABB2 SetViewport(AABB2 viewPort);

    /// Getter
    int GetControllerIndex();

public:
    Camera* m_viewCamera = nullptr; // Handle screen message and hud
    AABB2   m_screenViewport;
    AABB2   m_viewport; // viewport size

    Camera*     m_worldCamera = nullptr; // Our camera. Used as the world camera when rendering.
    Vec3        m_position; // 3D position, separate from our actor so that we have a transform for the free-fly camera, as a Vec3, in world units.
    EulerAngles m_orientation; // 3D orientation, separate from our actor so that we have a transform for the free-fly camera, as EulerAngles, in degrees.
};
