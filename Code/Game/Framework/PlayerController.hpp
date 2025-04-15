#pragma once
#include "Controller.hpp"
#include "../Entity.hpp"
#include "Engine/Core/EngineCommon.hpp"

class Actor;
class Camera;

/*
 2.	Free-fly camera should be able to move and rotate while the game is paused.
3.	When an actor is possessed.
a.	Movement control input should instruct the actor to move in direction at the appropriate speed based on whether we are sprinting or not.
b.	Set the orientation directly when aiming. Do not use angular velocity.
c.	When the fire button is pressed, instruct the actor to attack.
d.	When a weapon select button is pressed, instruct the actor to equip the appropriate weapon.
e.	Set the world camera to use the possessed actor's eye height and FOV.
f.	Print out player actor health each frame in the bottom middle portion of the screen.i.
g.	While the player actor is dead, animate the camera dropping to the ground7 such that it hits the ground halfway through the corpse lifetime.
h.	The player controller should not be able to move, aim, or fire while the actor is dead.
*/
class PlayerController : public Controller
{
public:
    PlayerController(Map* map);
    PlayerController(Map* map, Vec3 position, EulerAngles orientation = EulerAngles());
    ~PlayerController() override;

    void Possess(ActorHandle& actorHandle) override;

    void Update(float deltaSeconds) override;
    void UpdateWeapon(float deltaSeconds);
    void UpdateInput(float deltaSeconds); // Perform input processing for controlling actors and free-fly camera mode.
    void UpdateCamera(float deltaSeconds); // Update our camera settings, taking in to account actor eye height and field of vision.
    void Render() const;

    void HandleActorDead(float deltaSeconds);

    Mat44 GetModelToWorldTransform() const;

public:
    Camera*     m_camera = nullptr; // Our camera. Used as the world camera when rendering.
    Vec3        m_position; // 3D position, separate from our actor so that we have a transform for the free-fly camera, as a Vec3, in world units.
    EulerAngles m_orientation; // 3D orientation, separate from our actor so that we have a transform for the free-fly camera, as EulerAngles, in degrees.
    bool        m_bCameraMode = false; // Toggles whether we are controlling an actor or a free-fly camera currently.
    AABB2       m_viewport; // viewport size
private:
    float m_speed    = 2.0f;
    float m_turnRate = 0.075f;

    void HandleRayCast();
    void UpdateDebugMessage();
    /// 
};
