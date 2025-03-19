#include "Player.hpp"

#include "Game.hpp"
#include "GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRenderSystem.h"
#include "Engine/Renderer/Renderer.hpp"
#include "Gameplay/Map.hpp"

Player::Player(Game* owner)
{
    m_camera         = new Camera();
    m_camera->m_mode = eMode_Perspective;
    m_camera->SetOrthographicView(Vec2(-1, -1), Vec2(1, 1));
    m_speed    = g_gameConfigBlackboard.GetValue("playerSpeed", m_speed);
    m_turnRate = g_gameConfigBlackboard.GetValue("playerTurnRate", m_turnRate);
    printf("Object::Player    + Creating Player at (%f, %f, %f)\n", m_position.x, m_position.y, m_position.z);
}

Player::Player(Game* owner, Vec3 position, EulerAngles orientation): m_game(owner), m_position(position), m_orientation(orientation)
{
    m_camera         = new Camera();
    m_camera->m_mode = eMode_Perspective;
    m_camera->SetOrthographicView(Vec2(-1, -1), Vec2(1, 1));
    m_speed    = g_gameConfigBlackboard.GetValue("playerSpeed", m_speed);
    m_turnRate = g_gameConfigBlackboard.GetValue("playerTurnRate", m_turnRate);
    printf("Object::Player    + Creating Player at (%f, %f, %f)\n", m_position.x, m_position.y, m_position.z);
}

Player::~Player()
{
    printf("Object::Player    - Destroy Player and free resources\n");
    POINTER_SAFE_DELETE(m_camera)
}

Actor* Player::BindActorToPlayer(Actor* targetActor)
{
    m_testingActor = targetActor;
    return targetActor;
}

void Player::Update(float deltaSeconds)
{
    m_camera->SetPerspectiveView(2.0f, 60.f, 0.1f, 100.f);
    Mat44 ndcMatrix;
    ndcMatrix.SetIJK3D(Vec3(0, 0, 1), Vec3(-1, 0, 0), Vec3(0, 1, 0));
    m_camera->SetCameraToRenderTransform(ndcMatrix);

    m_camera->SetPosition(m_position);
    m_camera->SetOrientation(m_orientation);

    HandleRayCast();
    HandleControlledActorKey();

    Vec2 cursorDelta = g_theInput->GetCursorClientDelta();
    //printf(Stringf("%f %f \n", cursorDelta.x, cursorDelta.y).c_str());


    m_orientation.m_yawDegrees += -cursorDelta.x * 0.125f;
    m_orientation.m_pitchDegrees += -cursorDelta.y * 0.125f;

    const XboxController& controller = g_theInput->GetController(0);
    float                 speed      = m_speed;
    float                 turnRate   = m_turnRate;

    Vec2  leftStickPos  = controller.GetLeftStick().GetPosition();
    Vec2  rightStickPos = controller.GetRightStick().GetPosition();
    float leftStickMag  = controller.GetLeftStick().GetMagnitude();
    float rightStickMag = controller.GetRightStick().GetMagnitude();
    float leftTrigger   = controller.GetLeftTrigger();
    float rightTrigger  = controller.GetRightTrigger();

    if (rightStickMag > 0.f)
    {
        m_orientation.m_yawDegrees += -(rightStickPos * speed * rightStickMag * turnRate).x;
        m_orientation.m_pitchDegrees += -(rightStickPos * speed * rightStickMag * turnRate).y;
    }

    if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT) || controller.IsButtonDown(XBOX_BUTTON_A))
    {
        speed *= 15.f;
    }

    m_orientation.m_rollDegrees += leftTrigger * turnRate * deltaSeconds * speed;
    m_orientation.m_rollDegrees -= rightTrigger * turnRate * deltaSeconds * speed;


    if (g_theInput->IsKeyDown('Q'))
    {
        m_orientation.m_rollDegrees += turnRate;
    }

    if (g_theInput->IsKeyDown('E'))
    {
        m_orientation.m_rollDegrees -= turnRate;
    }


    //m_orientation.m_yawDegrees   = GetClamped(m_orientation.m_yawDegrees, -85.f, 85.f);
    m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);
    m_orientation.m_rollDegrees  = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);

    Vec3 forward, left, up;
    m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

    //printf("x: %f, y: %f\n", leftStickPos.x, leftStickPos.y);

    m_position += (leftStickPos * speed * leftStickMag * deltaSeconds).y * forward;
    m_position += -(leftStickPos * speed * leftStickMag * deltaSeconds).x * left;

    if (m_bIsControlledTestActor)
    {
        HandleControlledTestActor(deltaSeconds);
        return;
    }

    if (g_theInput->IsKeyDown('W'))
    {
        m_position += forward * speed * deltaSeconds;
    }

    if (g_theInput->IsKeyDown('S'))
    {
        m_position -= forward * speed * deltaSeconds;
    }

    if (g_theInput->IsKeyDown('A'))
    {
        m_position += left * speed * deltaSeconds;
    }

    if (g_theInput->IsKeyDown('D'))
    {
        m_position -= left * speed * deltaSeconds;
    }

    if (g_theInput->IsKeyDown('Z') || controller.IsButtonDown(XBOX_BUTTON_RS))
    {
        m_position.z -= deltaSeconds * speed;
    }

    if (g_theInput->IsKeyDown('C') || controller.IsButtonDown(XBOX_BUTTON_LS))
    {
        m_position.z += deltaSeconds * speed;
    }
}

void Player::Render() const
{
    g_theRenderer->BeingCamera(*m_camera);
    g_theRenderer->EndCamera(*m_camera);
}

Mat44 Player::GetModelToWorldTransform() const
{
    Mat44 matTranslation = Mat44::MakeTranslation3D(m_position);
    matTranslation.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
    matTranslation.Append(Mat44::MakeNonUniformScale3D(m_scale));
    return matTranslation;
}

void Player::HandleControlledActorKey()
{
    if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
    {
        m_bIsControlledTestActor = !m_bIsControlledTestActor;
    }
    AABB2 space;
    space.m_mins = Vec2::ZERO;
    space.m_maxs = space.m_mins + Vec2(400, 20);
    if (m_bIsControlledTestActor)
    {
        DebugAddScreenText("[F1] Control Mode:  Actor", space, 18, 0, Rgba8::BLUE, Rgba8::BLUE);
    }
    else
    {
        DebugAddScreenText("[F1] Control Mode: Camera", space, 18, 0);
    }
}

void Player::HandleControlledTestActor(float deltaSeconds)
{
    Vec3 forward, left, up;
    m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
    forward.z   = left.z = up.z = 0;
    float speed = m_speed;
    if (g_theInput->IsKeyDown('W'))
    {
        m_testingActor->m_position += forward * speed * deltaSeconds;
    }
    if (g_theInput->IsKeyDown('S'))
    {
        m_testingActor->m_position -= forward * speed * deltaSeconds;
    }
    if (g_theInput->IsKeyDown('A'))
    {
        m_testingActor->m_position += left * speed * deltaSeconds;
    }
    if (g_theInput->IsKeyDown('D'))
    {
        m_testingActor->m_position -= left * speed * deltaSeconds;
    }
    if (g_theInput->IsKeyDown('Z'))
    {
        m_testingActor->m_position.z -= deltaSeconds * speed;
    }

    if (g_theInput->IsKeyDown('C'))
    {
        m_testingActor->m_position.z += deltaSeconds * speed;
    }
}

void Player::HandleRayCast()
{
    if (g_theInput->WasMouseButtonJustPressed(KEYCODE_LEFT_MOUSE))
    {
        Vec3 forward, left, up;
        m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
        RaycastResult3D result = m_game->m_map->RaycastAll(m_position, forward, 10.f);
        if (result.m_didImpact)
        {
            DebugAddWorldCylinder(result.m_rayStartPos, result.m_rayStartPos + result.m_rayFwdNormal * 10, 0.01f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
            DebugAddWorldSphere(result.m_impactPos, 0.06f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::USE_DEPTH);
            DebugAddWorldArrow(result.m_impactPos + result.m_impactNormal * 0.3f, result.m_impactPos, 0.03f, 10.f, Rgba8(0, 6, 177), Rgba8(0, 6, 177));
        }
        else
        {
            DebugAddWorldCylinder(m_position, m_position + forward * 10, 0.01f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
        }
    }
    if (g_theInput->WasMouseButtonJustPressed(KEYCODE_RIGHT_MOUSE))
    {
        Vec3 forward, left, up;
        m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
        RaycastResult3D result = m_game->m_map->RaycastAll(m_position, forward, 0.5f);
        if (result.m_didImpact)
        {
            DebugAddWorldCylinder(result.m_rayStartPos, result.m_rayStartPos + result.m_rayFwdNormal * 0.5f, 0.01f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
            DebugAddWorldSphere(result.m_impactPos, 0.06f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::USE_DEPTH);
            DebugAddWorldArrow(result.m_impactPos + result.m_impactNormal * 0.3f, result.m_impactPos, 0.03f, 10.f, Rgba8(0, 6, 177), Rgba8(0, 6, 177));
        }
        else
        {
            DebugAddWorldCylinder(m_position, m_position + forward * 0.5f, 0.01f, 10.f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::X_RAY);
        }
    }
}
