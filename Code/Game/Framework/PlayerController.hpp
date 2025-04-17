#pragma once
#include "Controller.hpp"
#include "../Entity.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"

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
    ~PlayerController() override;

    void Possess(ActorHandle& actorHandle) override;

    /// Update
    void Update(float deltaSeconds) override;
    void UpdateWeapon(float deltaSeconds);
    void UpdateKeyboardInput(float deltaSeconds);
    void UpdateControllerInput(float deltaSeconds);
    void UpdateInput(float deltaSeconds); // Perform input processing for controlling actors and free-fly camera mode.
    void UpdateCamera(float deltaSeconds); // Update our camera settings, taking in to account actor eye height and field of vision.

    /// Render
    void Render() const;

    void HandleActorDead(float deltaSeconds);

    /// Setter
    DeviceType SetInputDeviceType(DeviceType newDeviceType);
    DeviceType GetInputDeviceType() const;
    Mat44      GetModelToWorldTransform() const;

public:
    bool m_bCameraMode = false; // Toggles whether we are controlling an actor or a free-fly camera currently.

private:
    float      m_speed      = 2.0f;
    float      m_turnRate   = 0.075f;
    DeviceType m_deviceType = DeviceType::KEYBOARD_AND_MOUSE;

    void HandleRayCast();
    void UpdateDebugMessage();
    /// 
};
