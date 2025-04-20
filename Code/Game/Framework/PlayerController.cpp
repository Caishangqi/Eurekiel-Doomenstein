#include "PlayerController.hpp"

#include "WidgetSubsystem.hpp"
#include "../GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "../Gameplay/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Definition/ActorDefinition.hpp"
#include "Game/Gameplay/Weapon.hpp"
#include "Game/Gameplay/Widget/WidgetPlayerDeath.hpp"


PlayerController::PlayerController(Map* map): Controller(map)
{
    m_worldCamera         = new Camera();
    m_worldCamera->m_mode = eMode_Perspective;
    m_worldCamera->SetOrthographicView(Vec2(-1, -1), Vec2(1, 1));
    m_viewCamera         = new Camera();
    m_viewCamera->m_mode = eMode_Orthographic;
    m_viewCamera->SetOrthographicView(Vec2::ZERO, g_theGame->m_screenSpace.m_maxs); // TODO: use the normalized viewport
    m_speed    = g_gameConfigBlackboard.GetValue("playerSpeed", m_speed);
    m_turnRate = g_gameConfigBlackboard.GetValue("playerTurnRate", m_turnRate);
    printf("Object::PlayerController    + Creating PlayerController at (%f, %f, %f)\n", m_position.x, m_position.y, m_position.z);
}


PlayerController::~PlayerController()
{
    printf("Object::PlayerController    - Destroy PlayerController and free resources\n");
    POINTER_SAFE_DELETE(m_worldCamera)
}


void PlayerController::Possess(ActorHandle& actorHandle)
{
    Controller::Possess(actorHandle);
}

void PlayerController::Update(float deltaSeconds)
{
    Controller::Update(deltaSeconds);
    HandleActorDead(deltaSeconds);
    UpdateDebugMessage();
    HandleRayCast();
    UpdateInput(deltaSeconds);
    UpdateWeapon(deltaSeconds);
    UpdateCamera(deltaSeconds);
}

void PlayerController::UpdateWeapon(float deltaSeconds)
{
    if (g_theGame->m_currentState != GameState::PLAYING)
        return;
    if (!m_actorHandle.IsValid())
        return;
    if (m_bCameraMode)
        return;
    Actor* possessActor = m_map->GetActorByHandle(m_actorHandle);
    if (possessActor && possessActor->m_definition->m_name == "Marine")
    {
        if (possessActor->m_currentWeapon)
            possessActor->m_currentWeapon->Update(deltaSeconds);
    }
}

void PlayerController::UpdateKeyboardInput(float deltaSeconds)
{
    UNUSED(deltaSeconds)
    Vec2 cursorDelta = g_theInput->GetCursorClientDelta();
    if (g_theInput->WasKeyJustPressed('N'))
    {
        if (m_map && g_theGame->GetIsSingleMode())
            m_map->DebugPossessNext();
    }
    if (g_theInput->WasKeyJustPressed('F'))
    {
        if (!g_theGame->GetIsSingleMode())
        {
            printf("PlayerController::UpdateKeyboardInput       Free Camera mode is disable in Multiplayer\n");
        }
        else { m_bCameraMode = !m_bCameraMode; }
    }

    if (g_theGame->m_currentState != GameState::PLAYING)
        return;

    if (!m_bCameraMode)
    {
        Actor* possessActor = GetActor();
        if (!possessActor)
            return;
        if (possessActor->m_bIsDead) /// Handle player Actor dead
            return;
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

        if (g_theGame->m_currentState == GameState::PLAYING)
        {
            if (g_theInput->WasMouseButtonJustPressed(KEYCODE_LEFT_MOUSE) && !possessActor->m_bIsDead)
                GetActor()->m_currentWeapon->Fire();
        }

        if (g_theInput->IsKeyDown('W'))
        {
            possessActor->MoveInDirection(forward, actorSpeed);
            possessActor->PlayAnimationByName("Walk");
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
            possessActor->EquipWeapon(0);
        }
        if (g_theInput->WasKeyJustPressed('2'))
        {
            possessActor->EquipWeapon(1);
        }
        if (g_theInput->WasKeyJustPressed('3'))
        {
            possessActor->EquipWeapon(2);
        }
        if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTARROW))
        {
            auto it    = std::find(possessActor->m_weapons.begin(), possessActor->m_weapons.end(), possessActor->m_currentWeapon);
            int  index = static_cast<int>(it - possessActor->m_weapons.begin());
            index--;
            int newIndex = static_cast<int>((index + possessActor->m_weapons.size()) % possessActor->m_weapons.size());
            possessActor->EquipWeapon(newIndex);
        }
        if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTARROW))
        {
            auto it    = std::find(possessActor->m_weapons.begin(), possessActor->m_weapons.end(), possessActor->m_currentWeapon);
            int  index = static_cast<int>(it - possessActor->m_weapons.begin());
            index++;
            int newIndex = index % possessActor->m_weapons.size();
            possessActor->EquipWeapon(newIndex);
        }


        m_position    = possessActor->m_position;
        m_orientation = possessActor->m_orientation;
    }
    else
    {
        m_orientation.m_yawDegrees += -cursorDelta.x * 0.125f;
        m_orientation.m_pitchDegrees += -cursorDelta.y * 0.125f;


        m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);
        m_orientation.m_rollDegrees  = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);

        Vec3 forward, left, up;
        m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

        float speed = m_speed;

        if (g_theInput->IsKeyDown(SHIFT_PRESSED))
        {
            speed *= 15.0f;
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

        if (g_theInput->IsKeyDown('Z'))
        {
            m_position.z -= deltaSeconds * speed;
        }

        if (g_theInput->IsKeyDown('C'))
        {
            m_position.z += deltaSeconds * speed;
        }
    }
}

void PlayerController::UpdateControllerInput(float deltaSeconds)
{
    if (g_theGame->m_currentState != GameState::PLAYING)
        return;

    const XboxController& controller = g_theInput->GetController(0);
    float                 speed      = m_speed;
    float                 turnRate   = m_turnRate;
    if (!m_bCameraMode)
    {
        Actor* possessActor = GetActor();
        if (!possessActor)
            return;
        if (possessActor->m_bIsDead) /// Handle player Actor dead
            return;
        EulerAngles possessActorOrientation = possessActor->m_orientation;

        float actorSpeed = possessActor->m_definition->m_walkSpeed;

        Vec2  leftStickPos  = controller.GetLeftStick().GetPosition();
        Vec2  rightStickPos = controller.GetRightStick().GetPosition();
        float leftStickMag  = controller.GetLeftStick().GetMagnitude();
        float rightStickMag = controller.GetRightStick().GetMagnitude();
        float leftTrigger   = controller.GetLeftTrigger();
        float rightTrigger  = controller.GetRightTrigger();


        if (rightStickMag > 0.f)
        {
            turnRate = possessActor->m_definition->m_turnSpeed;
            possessActorOrientation.m_yawDegrees += -(rightStickPos * speed * rightStickMag * turnRate * deltaSeconds).x;
            possessActorOrientation.m_pitchDegrees += -(rightStickPos * speed * rightStickMag * turnRate * deltaSeconds).y;
            possessActor->TurnInDirection(Vec3(possessActorOrientation));
        }

        if (controller.IsButtonDown(XBOX_BUTTON_A))
        {
            actorSpeed = possessActor->m_definition->m_runSpeed;
        }

        Vec3 forward, left, up;
        possessActor->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
        if (leftStickMag > 0.f)
        {
            // Combine X / Y stick input into one movement vector
            Vec3 moveDir = forward * leftStickPos.y + -left * leftStickPos.x;
            moveDir.z    = 0.f;
            possessActor->MoveInDirection(moveDir.GetNormalized(), actorSpeed);
            possessActor->PlayAnimationByName("Walk");
        }

        if (controller.WasButtonJustPressed(XBOX_BUTTON_DPAD_DOWN))
        {
            auto it    = std::find(possessActor->m_weapons.begin(), possessActor->m_weapons.end(), possessActor->m_currentWeapon);
            int  index = static_cast<int>(it - possessActor->m_weapons.begin());
            index--;
            int newIndex = static_cast<int>((index + possessActor->m_weapons.size()) % possessActor->m_weapons.size());
            possessActor->EquipWeapon(newIndex);
        }
        if (rightTrigger > 0.f)
        {
            GetActor()->m_currentWeapon->Fire();
        }
        if (controller.WasButtonJustPressed(XBOX_BUTTON_Y))
        {
            possessActor->EquipWeapon(1);
        }
        if (controller.WasButtonJustPressed(XBOX_BUTTON_X))
        {
            possessActor->EquipWeapon(0);
        }
        if (controller.WasButtonJustPressed(XBOX_BUTTON_DPAD_UP))
        {
            auto it    = std::find(possessActor->m_weapons.begin(), possessActor->m_weapons.end(), possessActor->m_currentWeapon);
            int  index = static_cast<int>(it - possessActor->m_weapons.begin());
            index++;
            int newIndex = index % possessActor->m_weapons.size();
            possessActor->EquipWeapon(newIndex);
        }

        m_orientation.m_rollDegrees += leftTrigger * turnRate * deltaSeconds * speed;
        m_orientation.m_rollDegrees -= rightTrigger * turnRate * deltaSeconds * speed;
    }
}

void PlayerController::UpdateInput(float deltaSeconds)
{
    UNUSED(deltaSeconds)
    Vec2 cursorDelta = g_theInput->GetCursorClientDelta();

    switch (m_deviceType)
    {
    case DeviceType::CONTROLLER:
        {
            UpdateControllerInput(deltaSeconds);
            break;
        }
    case DeviceType::KEYBOARD_AND_MOUSE:
        {
            UpdateKeyboardInput(deltaSeconds);
            break;
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
        if (possessActor && !possessActor->m_bIsDead)
        {
            m_worldCamera->SetPerspectiveView(m_worldCamera->GetViewPortUnnormalizedAspectRatio(Vec2(g_theGame->m_screenSpace.GetDimensions().x, g_theGame->m_screenSpace.GetDimensions().y)),
                                              GetActor()->m_definition->m_cameraFOV, 0.1f, 100.f);
            // Set the world camera to use the possessed actor's eye height and FOV.
            m_position    = Vec3(possessActor->m_position.x, possessActor->m_position.y, possessActor->m_definition->m_eyeHeight);
            m_orientation = possessActor->m_orientation;
        }
        else
        {
            m_worldCamera->SetPerspectiveView(m_worldCamera->GetViewPortUnnormalizedAspectRatio(Vec2(g_theGame->m_screenSpace.GetDimensions().x, g_theGame->m_screenSpace.GetDimensions().y)), 60.f,
                                              0.1f, 100.f);
        }
    }
    //m_viewCamera->SetOrthographicView(m_screenViewport.m_mins, m_screenViewport.m_maxs);
    m_viewCamera->SetOrthographicView(g_theGame->m_screenSpace.m_mins, g_theGame->m_screenSpace.m_maxs);
    m_worldCamera->SetPosition(m_position);
    m_worldCamera->SetOrientation(m_orientation);
    Mat44 ndcMatrix;
    ndcMatrix.SetIJK3D(Vec3(0, 0, 1), Vec3(-1, 0, 0), Vec3(0, 1, 0));
    m_worldCamera->SetCameraToRenderTransform(ndcMatrix);
}

void PlayerController::Render() const
{
    g_theRenderer->BeingCamera(*m_viewCamera);
    if (g_theGame->m_currentState != GameState::PLAYING)
        return;
    if (!m_actorHandle.IsValid())
        return;
    if (m_bCameraMode)
        return;
    Actor* possessActor = m_map->GetActorByHandle(m_actorHandle);
    if (possessActor->m_definition->m_name == "Marine")
    {
        if (possessActor->m_currentWeapon)
            possessActor->m_currentWeapon->Render();
    }
    g_theRenderer->EndCamera(*m_viewCamera);
}

void PlayerController::HandleActorDead(float deltaSeconds)
{
    UNUSED(deltaSeconds)
    if (g_theGame->m_currentState != GameState::PLAYING)
        return;
    if (m_actorHandle.IsValid())
    {
        Actor* possessActor = m_map->GetActorByHandle(m_actorHandle);
        if (possessActor && possessActor->m_bIsDead && possessActor->m_definition->m_name == "Marine")
        {
            if (possessActor->m_dead <= deltaSeconds)
            {
                auto playerDeathWidget = new WidgetPlayerDeath();
                g_theWidgetSubsystem->AddToPlayerViewport(playerDeathWidget, this);
            }
            Vec3  startPos      = possessActor->m_position + Vec3(0, 0, possessActor->m_definition->m_eyeHeight);
            Vec3  endPos        = possessActor->m_position;
            float deathFraction = GetClamped(possessActor->m_dead / possessActor->m_definition->m_corpseLifetime, 0.f, 1.f);
            float interpolate   = Interpolate(startPos.z, endPos.z, deathFraction);
            m_position          = Vec3(possessActor->m_position.x, possessActor->m_position.y, interpolate);
        }
    }
}

DeviceType PlayerController::SetInputDeviceType(DeviceType newDeviceType)
{
    m_deviceType = newDeviceType;
    return newDeviceType;
}

DeviceType PlayerController::GetInputDeviceType() const
{
    return m_deviceType;
}

Mat44 PlayerController::GetModelToWorldTransform() const
{
    Mat44 matTranslation = Mat44::MakeTranslation3D(m_position);
    matTranslation.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
    return matTranslation;
}

void PlayerController::HandleRayCast()
{
}

void PlayerController::UpdateDebugMessage()
{
    if (g_theGame->m_currentState == GameState::PLAYING)
    {
        AABB2 size    = g_theGame->m_screenSpace;
        size.m_maxs.y = size.m_mins.y + 60;
        size.m_maxs.x /= 2;
        size.m_maxs.x += 200;
        // Actor* possessActor = GetActor();
        /*if (possessActor)
            DebugAddScreenText(Stringf("Possessor Health: %.1f", possessActor->m_health), size, 18.f, 0);*/
    }
}
