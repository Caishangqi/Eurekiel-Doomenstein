#include "PlayerController.hpp"

#include "../GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRenderSystem.h"
#include "Engine/Renderer/Renderer.hpp"
#include "../Gameplay/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Definition/ActorDefinition.hpp"
#include "Game/Gameplay/Weapon.hpp"

PlayerController::PlayerController(Map* map): Controller(map)
{
    m_camera         = new Camera();
    m_camera->m_mode = eMode_Perspective;
    m_camera->SetOrthographicView(Vec2(-1, -1), Vec2(1, 1));
    m_speed    = g_gameConfigBlackboard.GetValue("playerSpeed", m_speed);
    m_turnRate = g_gameConfigBlackboard.GetValue("playerTurnRate", m_turnRate);
    printf("Object::PlayerController    + Creating PlayerController at (%f, %f, %f)\n", m_position.x, m_position.y, m_position.z);
}

PlayerController::PlayerController(Map* map, Vec3 position, EulerAngles orientation): Controller(map), m_position(position), m_orientation(orientation)
{
    m_camera         = new Camera();
    m_camera->m_mode = eMode_Perspective;
    m_camera->SetOrthographicView(Vec2(-1, -1), Vec2(1, 1));
    m_speed    = g_gameConfigBlackboard.GetValue("playerSpeed", m_speed);
    m_turnRate = g_gameConfigBlackboard.GetValue("playerTurnRate", m_turnRate);
    printf("Object::PlayerController    + Creating PlayerController at (%f, %f, %f)\n", m_position.x, m_position.y, m_position.z);
}

PlayerController::~PlayerController()
{
    printf("Object::PlayerController    - Destroy PlayerController and free resources\n");
    POINTER_SAFE_DELETE(m_camera)
}

void PlayerController::Possess(ActorHandle& actorHandle)
{
    Controller::Possess(actorHandle);
    Actor* possessActor = m_map->GetActorByHandle(actorHandle);
    // Set the world camera to use the possessed actor's eye height and FOV.
    m_position    = Vec3(possessActor->m_position.x, possessActor->m_position.y, possessActor->m_definition->m_eyeHeight);
    m_orientation = possessActor->m_orientation;
}

void PlayerController::Update(float deltaSeconds)
{
    Controller::Update(deltaSeconds);

    HandleRayCast();
    UpdateInput(deltaSeconds);
    UpdateCamera(deltaSeconds);
}

void PlayerController::UpdateInput(float deltaSeconds)
{
    UNUSED(deltaSeconds)
    Vec2 cursorDelta = g_theInput->GetCursorClientDelta();

    const XboxController& controller = g_theInput->GetController(0);
    float                 speed      = m_speed;
    float                 turnRate   = m_turnRate;

    if (g_theInput->WasKeyJustPressed('N'))
    {
        if (m_map)
            m_map->DebugPossessNext();
    }
    if (g_theInput->WasKeyJustPressed('F'))
    {
        m_bCameraMode = !m_bCameraMode;
    }

    if (g_theGame->m_currentState != GameState::PLAYING)
        return;

    if (!m_bCameraMode)
    {
        Actor*      possessActor            = GetActor();
        EulerAngles possessActorOrientation = possessActor->m_orientation;
        float       actorSpeed              = possessActor->m_definition->m_walkSpeed;
        if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT))
        {
            actorSpeed = possessActor->m_definition->m_runSpeed;
        }
        possessActorOrientation.m_yawDegrees += -cursorDelta.x * 0.125f;
        possessActorOrientation.m_pitchDegrees += -cursorDelta.y * 0.125f;

        possessActor->TurnInDirection(Vec3(possessActorOrientation));

        Vec3 forward, left, up;
        possessActor->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

        if (g_theInput->IsKeyDown('W'))
        {
            possessActor->MoveInDirection(forward, actorSpeed);
        }

        if (g_theInput->IsKeyDown('S'))
        {
            possessActor->MoveInDirection(-forward, actorSpeed);
        }

        if (g_theInput->IsKeyDown('A'))
        {
            possessActor->MoveInDirection(left, actorSpeed);
        }

        if (g_theInput->IsKeyDown('D'))
        {
            possessActor->MoveInDirection(-left, actorSpeed);
        }

        if (g_theInput->WasKeyJustPressed('1'))
        {
            possessActor->SwitchInventory(0);
        }
        if (g_theInput->WasKeyJustPressed('2'))
        {
            possessActor->SwitchInventory(1);
        }
        if (g_theInput->WasKeyJustPressed('3'))
        {
            possessActor->SwitchInventory(2);
        }
    }
    else
    {
        m_orientation.m_yawDegrees += -cursorDelta.x * 0.125f;
        m_orientation.m_pitchDegrees += -cursorDelta.y * 0.125f;

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
}

void PlayerController::UpdateCamera(float deltaSeconds)
{
    UNUSED(deltaSeconds)
    if (g_theGame->m_currentState != GameState::PLAYING)
        return;
    if (!m_bCameraMode)
    {
        Actor* possessActor = GetActor();
        if (possessActor)
        {
            m_camera->SetPerspectiveView(2.0f, GetActor()->m_definition->m_cameraFOV, 0.1f, 100.f);
            m_position    = Vec3(possessActor->m_position.x, possessActor->m_position.y, possessActor->m_definition->m_eyeHeight);
            m_orientation = possessActor->m_orientation;
        }
        else
        {
            m_camera->SetPerspectiveView(2.0f, 60.f, 0.1f, 100.f);
        }
    }
    m_camera->SetPosition(m_position);
    m_camera->SetOrientation(m_orientation);
    Mat44 ndcMatrix;
    ndcMatrix.SetIJK3D(Vec3(0, 0, 1), Vec3(-1, 0, 0), Vec3(0, 1, 0));
    m_camera->SetCameraToRenderTransform(ndcMatrix);
}

void PlayerController::Render() const
{
    g_theRenderer->BeingCamera(*m_camera);
    g_theRenderer->EndCamera(*m_camera);
}

Mat44 PlayerController::GetModelToWorldTransform() const
{
    Mat44 matTranslation = Mat44::MakeTranslation3D(m_position);
    matTranslation.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
    return matTranslation;
}

void PlayerController::HandleRayCast()
{
    if (g_theGame->m_currentState == GameState::PLAYING && !m_bCameraMode)
    {
        if (g_theInput->WasMouseButtonJustPressed(KEYCODE_LEFT_MOUSE))
            GetActor()->m_currentWeapon->Fire();
    }
    if (g_theGame->m_currentState != GameState::PLAYING)
        return;
    /*if (g_theInput->WasMouseButtonJustPressed(KEYCODE_LEFT_MOUSE))
    {
        Vec3 forward, left, up;
        m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
        RaycastResult3D result = m_map->RaycastAll(m_position, forward, 10.f);
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
        RaycastResult3D result = m_map->RaycastAll(m_position, forward, 0.5f);
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
    }*/
}
