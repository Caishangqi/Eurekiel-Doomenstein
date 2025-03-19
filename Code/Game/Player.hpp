#pragma once
#include "Entity.hpp"

class Actor;
class Camera;

class Player
{
public:
    Player(Game* owner);
    Player(Game* owner, Vec3 position, EulerAngles orientation = EulerAngles());
    ~Player();

    /// Testing Only
    Actor* BindActorToPlayer(Actor* targetActor);
    /// 

    void Update(float deltaSeconds);
    void Render() const;

    Mat44 GetModelToWorldTransform() const;

public:
    Game*   m_game   = nullptr;
    Camera* m_camera = nullptr;

    Vec3        m_position;
    Vec3        m_velocity;
    EulerAngles m_orientation;
    Vec3        m_scale = Vec3(1, 1, 1);
    EulerAngles m_angularVelocity;

    Rgba8 m_color = Rgba8::WHITE;

private:
    float m_speed    = 2.0f;
    float m_turnRate = 0.075f;

    /// Testing Only
    Actor* m_testingActor           = nullptr;
    bool   m_bIsControlledTestActor = false;
    void   HandleControlledActorKey();
    void   HandleControlledTestActor(float deltaSeconds);
    void   HandleRayCast();
    /// 
};
