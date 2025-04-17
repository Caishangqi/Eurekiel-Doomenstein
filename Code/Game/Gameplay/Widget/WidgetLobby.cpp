#include "WidgetLobby.hpp"

#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/Gameplay/Save/PlayerSaveSubsystem.hpp"

class XboxController;

WidgetLobby::WidgetLobby()
{
}

WidgetLobby::~WidgetLobby()
{
}

void WidgetLobby::Draw() const
{
    Widget::Draw();
    g_theRenderer->ClearScreen(g_theApp->m_backgroundColor);
    g_theRenderer->BindTexture(nullptr);
    if (g_theGame->m_localPlayerControllers.size() == 1)
    {
        PlayerController*       controller = g_theGame->m_localPlayerControllers[0];
        std::vector<Vertex_PCU> textVerts;
        textVerts.reserve(4096);
        /// Render One Player
        AABB2       bound      = g_theGame->m_screenSpace;
        BitmapFont* g_testFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
        textVerts.clear();
        g_testFont->AddVertsForTextInBox2D(textVerts, Stringf("Player %d", controller->GetControllerIndex()), bound, 55.f, Rgba8::WHITE, 1, Vec2(0.5f, 0.6f));
        g_testFont->AddVertsForTextInBox2D(textVerts, Stringf("%s", to_string(controller->GetInputDeviceType())), bound, 25.f, Rgba8::WHITE, 1, Vec2(0.5f, 0.5f));

        std::vector<std::string> mappingTexts = GetPlayerActionNamesByDeviceType(controller->GetInputDeviceType());
        bound.m_maxs.y -= 400.f;
        g_testFont->AddVertsForTextInBox2D(textVerts, Stringf("Press %s to start game", mappingTexts[0].c_str()), bound, 20.f, Rgba8::WHITE, 1, Vec2(0.5f, 0.5f));
        bound.m_maxs.y -= 60.f;
        g_testFont->AddVertsForTextInBox2D(textVerts, Stringf("Press %s to leave game", mappingTexts[1].c_str()), bound, 20.f, Rgba8::WHITE, 1, Vec2(0.5f, 0.5f));
        bound.m_maxs.y -= 60.f;
        g_testFont->AddVertsForTextInBox2D(textVerts, Stringf("Press %s to join game", mappingTexts[2].c_str()), bound, 20.f, Rgba8::WHITE, 1, Vec2(0.5f, 0.5f));

        g_theRenderer->BindTexture(&g_testFont->GetTexture());
        g_theRenderer->DrawVertexArray(textVerts);
    }
    else if (g_theGame->m_localPlayerControllers.size() == 2)
    {
        /// Render Two Player
        AABB2                   bound = g_theGame->m_screenSpace;
        std::vector<Vertex_PCU> textVerts;
        textVerts.reserve(4096);
        AABB2             bottomBound             = AABB2(Vec2(0.f, 0.f), Vec2(bound.m_maxs.x, bound.m_maxs.y / 2.f));
        AABB2             topBound                = AABB2(Vec2(0.f, bound.m_maxs.y / 2.f), Vec2(bound.m_maxs.x, bound.m_maxs.y));
        BitmapFont*       g_testFont              = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
        PlayerController* topDisplayController    = g_theGame->GetLocalPlayer(1);
        PlayerController* bottomDisplayController = g_theGame->GetLocalPlayer(2);
        std::string       startGameMappingText, leaveGameMappingText, joinPlayerMappingText;

        /// Handle top
        std::vector<std::string> mappingTextsTop = GetPlayerActionNamesByDeviceType(topDisplayController->GetInputDeviceType());
        g_testFont->AddVertsForTextInBox2D(textVerts, Stringf("Player %d", topDisplayController->GetControllerIndex()), topBound, 55.f, Rgba8::WHITE, 1, Vec2(0.5f, 0.6f));
        topBound.m_maxs.y -= 50.f;
        g_testFont->AddVertsForTextInBox2D(textVerts, Stringf("%s", to_string(topDisplayController->GetInputDeviceType())), topBound, 25.f, Rgba8::WHITE, 1, Vec2(0.5f, 0.5f));
        topBound.m_maxs.y -= 200.f;
        g_testFont->AddVertsForTextInBox2D(textVerts, Stringf("Press %s to start game", mappingTextsTop[0].c_str()), topBound, 15.f, Rgba8::WHITE, 1, Vec2(0.5f, 0.5f));
        topBound.m_maxs.y -= 60.f;
        g_testFont->AddVertsForTextInBox2D(textVerts, Stringf("Press %s to leave game", mappingTextsTop[1].c_str()), topBound, 15.f, Rgba8::WHITE, 1, Vec2(0.5f, 0.5f));

        /// Handle bottom
        std::vector<std::string> mappingTextsBottom = GetPlayerActionNamesByDeviceType(bottomDisplayController->GetInputDeviceType());
        g_testFont->AddVertsForTextInBox2D(textVerts, Stringf("Player %d", bottomDisplayController->GetControllerIndex()), bottomBound, 55.f, Rgba8::WHITE, 1, Vec2(0.5f, 0.6f));
        bottomBound.m_maxs.y -= 50.f;
        g_testFont->AddVertsForTextInBox2D(textVerts, Stringf("%s", to_string(bottomDisplayController->GetInputDeviceType())), bottomBound, 25.f, Rgba8::WHITE, 1, Vec2(0.5f, 0.5f));
        bottomBound.m_maxs.y -= 200.f;
        g_testFont->AddVertsForTextInBox2D(textVerts, Stringf("Press %s to start game", mappingTextsBottom[0].c_str()), bottomBound, 15.f, Rgba8::WHITE, 1, Vec2(0.5f, 0.5f));
        bottomBound.m_maxs.y -= 60.f;
        g_testFont->AddVertsForTextInBox2D(textVerts, Stringf("Press %s to leave game", mappingTextsBottom[1].c_str()), bottomBound, 15.f, Rgba8::WHITE, 1, Vec2(0.5f, 0.5f));

        /// Common
        g_theRenderer->BindTexture(&g_testFont->GetTexture());
        g_theRenderer->DrawVertexArray(textVerts);
    }
}

void WidgetLobby::Update()
{
    Widget::Update();
    UpdateKeyInput();
}


void WidgetLobby::UpdateKeyInput()
{
    const XboxController& controller          = g_theInput->GetController(0);
    bool                  spaceButtonPressed  = g_theInput->WasKeyJustPressed(32);
    bool                  escapeButtonPressed = g_theInput->WasKeyJustPressed(KEYCODE_ESC);
    bool                  startButtonPressed  = controller.WasButtonJustPressed(XBOX_BUTTON_START);
    bool                  backButtonPressed   = controller.WasButtonJustPressed(XBOX_BUTTON_BACK);
    if (g_theGame->m_localPlayerControllers.size() == 0)
        return;
    if (g_theGame->m_localPlayerControllers.size() == 1)
    {
        DeviceType deviceType      = g_theGame->m_localPlayerControllers[0]->GetInputDeviceType();
        int        controllerIndex = g_theGame->m_localPlayerControllers[0]->GetControllerIndex();
        if (deviceType == DeviceType::KEYBOARD_AND_MOUSE)
        {
            if (startButtonPressed)
            {
                if (controllerIndex == 1)
                    g_theGame->CreateLocalPlayer(2, DeviceType::CONTROLLER);
                else if (controllerIndex == 2)
                    g_theGame->CreateLocalPlayer(1, DeviceType::CONTROLLER);
            }
            else if (spaceButtonPressed)
            {
                HandleGameStartProcess();
                RemoveFromViewport();
            }
            else if (escapeButtonPressed)
            {
                g_theGame->EnterState(GameState::ATTRACT);
                RemoveAllLocalPlayerControllers();
                RemoveFromViewport();
            }
        }
        else if (deviceType == DeviceType::CONTROLLER)
        {
            if (spaceButtonPressed)
            {
                if (controllerIndex == 1)
                    g_theGame->CreateLocalPlayer(2, DeviceType::KEYBOARD_AND_MOUSE);
                else if (controllerIndex == 2)
                    g_theGame->CreateLocalPlayer(1, DeviceType::KEYBOARD_AND_MOUSE);
            }
            else if (startButtonPressed)
            {
                HandleGameStartProcess();
                RemoveFromViewport();
            }
            else if (backButtonPressed)
            {
                g_theGame->EnterState(GameState::ATTRACT);
                RemoveAllLocalPlayerControllers();
                RemoveFromViewport();
            }
        }
    }
    else if (g_theGame->m_localPlayerControllers.size() == 2)
    {
        if (startButtonPressed)
        {
            HandleGameStartProcess();
            RemoveFromViewport();
        }
        if (spaceButtonPressed)
        {
            HandleGameStartProcess();
            RemoveFromViewport();
        }
        if (escapeButtonPressed)
        {
            PlayerController* keyboardController = g_theGame->GetControllerByDeviceType(DeviceType::KEYBOARD_AND_MOUSE);
            g_theGame->RemoveLocalPlayer(keyboardController->GetControllerIndex());
        }
        else if (backButtonPressed)
        {
            PlayerController* gamepadController = g_theGame->GetControllerByDeviceType(DeviceType::CONTROLLER);
            g_theGame->RemoveLocalPlayer(gamepadController->GetControllerIndex());
        }
    }
}

void WidgetLobby::RemoveAllLocalPlayerControllers()
{
    g_theGame->m_localPlayerControllers.clear();
    g_theGame->m_localPlayerControllers.shrink_to_fit();
}

void WidgetLobby::HandleLocalPlayerViewportData()
{
    printf("WidgetLobby::HandleLocalPlayerViewportData      Update Player Controller Data\n");
    AABB2 clientSize = g_theGame->m_screenSpace;
    if (g_theGame->m_localPlayerControllers.size() == 1)
    {
        PlayerController* playerController = g_theGame->m_localPlayerControllers[0];
        playerController->SetViewport(AABB2::ZERO_TO_ONE);
    }
    else if (g_theGame->m_localPlayerControllers.size() == 2)
    {
        PlayerController* playerController1 = g_theGame->GetLocalPlayer(1);
        playerController1->SetViewport(AABB2(Vec2(0, 0.5f), Vec2::ONE));
        PlayerController* playerController2 = g_theGame->GetLocalPlayer(2);
        playerController2->SetViewport(AABB2(Vec2::ZERO, Vec2(1.0f, 0.5f)));
        /*playerController1->SetViewport(AABB2(Vec2(0, clientSize.m_maxs.y / 2.0f), Vec2(clientSize.m_maxs.x, clientSize.m_maxs.y)));
        playerController2->SetViewport(AABB2(Vec2::ZERO, Vec2(clientSize.m_maxs.x, clientSize.m_maxs.y / 2.0f)));*/
    }
}

void WidgetLobby::HandleGameStartProcess()
{
    printf("WidgetLobby::HandleGameStartProcess     Preparing resource for game.\n");
    HandleLocalPlayerViewportData();
    for (PlayerController* controller : g_theGame->m_localPlayerControllers)
    {
        PlayerSaveData save = {};
        save.m_playerID     = controller->m_index;
        PlayerSaveSubsystem::CreatePlayerSaveData(save);
    }
    g_theGame->EnterState(GameState::PLAYING);
}

std::vector<std::string> WidgetLobby::GetPlayerActionNamesByDeviceType(DeviceType deviceType) const
{
    std::string startGameMappingText, leaveGameMappingText, joinPlayerMappingText;
    switch (deviceType)
    {
    case DeviceType::KEYBOARD_AND_MOUSE:
        {
            startGameMappingText  = "SPACE";
            leaveGameMappingText  = "ESCAPE";
            joinPlayerMappingText = "START";
            break;
        }
    case DeviceType::CONTROLLER:
        {
            startGameMappingText  = "START";
            leaveGameMappingText  = "BACK";
            joinPlayerMappingText = "SPACE";
            break;
        }
    }
    std::vector<std::string> temp;
    temp.push_back(startGameMappingText);
    temp.push_back(leaveGameMappingText);
    temp.push_back(joinPlayerMappingText);
    return temp;
}
