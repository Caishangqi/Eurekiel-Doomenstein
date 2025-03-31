#include <Game/Game.hpp>

#include "App.hpp"
#include "GameCommon.hpp"
#include "Framework/PlayerController.hpp"
#include "Prop.hpp"
#include "Definition/ActorDefinition.hpp"
#include "Definition/MapDefinition.hpp"
#include "Definition/TileDefinition.hpp"
#include "Definition/WeaponDefinition.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRenderSystem.h"
#include "Engine/Renderer/Renderer.hpp"
#include "Gameplay/Map.hpp"

Game::Game()
{
    MapDefinition::LoadDefinitions("Data/Definitions/MapDefinitions.xml");
    TileDefinition::LoadDefinitions("Data/Definitions/TileDefinitions.xml");
    ActorDefinition::LoadDefinitions("Data/Definitions/ActorDefinitions.xml");
    ActorDefinition::LoadDefinitions("Data/Definitions/ProjectileActorDefinitions.xml");
    WeaponDefinition::LoadDefinitions("Data/Definitions/WeaponDefinitions.xml");

    /// Event Register
    g_theEventSystem->SubscribeEventCallbackFunction("GameStartEvent", GameStartEvent);
    g_theEventSystem->SubscribeEventCallbackFunction("GameExitEvent", GameExitEvent);
    /// 

    /// Resource
    g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");
    g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png");
    g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Caizii.png");

    /// Rasterize
    g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);

    /// Spaces
    m_screenSpace.m_mins = Vec2::ZERO;
    m_screenSpace.m_maxs = Vec2(g_gameConfigBlackboard.GetValue("screenSizeX", 1600.f), g_gameConfigBlackboard.GetValue("screenSizeY", 800.f));
    m_worldSpace.m_mins  = Vec2::ZERO;
    m_worldSpace.m_maxs  = Vec2(g_gameConfigBlackboard.GetValue("worldSizeX", 200.f), g_gameConfigBlackboard.GetValue("worldSizeY", 100.f));

    /// Cameras
    m_screenCamera         = new Camera();
    m_screenCamera->m_mode = eMode_Orthographic;
    m_screenCamera->SetOrthographicView(Vec2::ZERO, m_screenSpace.m_maxs);

    m_worldCamera         = new Camera();
    m_worldCamera->m_mode = eMode_Perspective;
    m_worldCamera->SetOrthographicView(Vec2(-1, -1), Vec2(1, 1));
    m_worldCamera->SetPerspectiveView(2.0f, 60.f, 0.1f, 100.f);
    Mat44 ndcMatrix;
    ndcMatrix.SetIJK3D(Vec3(0, 0, 1), Vec3(-1, 0, 0), Vec3(0, 1, 0));

    ///

    /// Clock
    m_clock = new Clock(Clock::GetSystemClock());
    ///

    /// Cube
    m_cube   = new Prop(this);
    m_cube_1 = new Prop(this);

    AddVertsForCube3D(m_cube->m_vertexes, Rgba8(255, 0, 0), Rgba8(0, 255, 255), Rgba8(0, 255, 0), Rgba8(255, 0, 255), Rgba8(0, 0, 255), Rgba8(255, 255, 0));
    AddVertsForCube3D(m_cube_1->m_vertexes, Rgba8(255, 0, 0), Rgba8(0, 255, 255), Rgba8(0, 255, 0), Rgba8(255, 0, 255), Rgba8(0, 0, 255), Rgba8(255, 255, 0));

    m_cube->m_position   = Vec3(2, 2, 0);
    m_cube_1->m_position = Vec3(-2, -2, 0);
    ///

    /// Test Prop
    m_testProp             = new Prop(this);
    m_testProp->m_position = Vec3(0, 0, 0);
    AddVertsForArrow3D(m_testProp->m_vertexes, Vec3(0, 2, 0), Vec3(0, 0, 0), 0.1f, 0.4f);
    /// 

    /// Ball
    m_ball             = new Prop(this);
    m_ball->m_position = Vec3(10, -5, 1);
    m_ball->m_texture  = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");
    //m_ball->m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Caizii.png");
    AddVertsForSphere3D(m_ball->m_vertexes, Vec3(0, 0, 0), 2, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 64, 32);
    /// 

    /// Grid
    m_grid_x = new Prop(this);
    AddVertsForCube3D(m_grid_x->m_vertexes, Rgba8::RED);
    m_grid_x->m_scale = Vec3(GRID_SIZE * 2.f, 0.1f, 0.1f);
    m_grid_y          = new Prop(this);

    AddVertsForCube3D(m_grid_y->m_vertexes, Rgba8::GREEN);
    m_grid_y->m_scale = Vec3(0.1f, GRID_SIZE * 2.f, 0.1f);

    m_grid_x_unit_5.resize(GRID_SIZE * 2 / GRID_UNIT_SIZE + 1);
    for (int i = 0; i < GRID_SIZE * 2 / GRID_UNIT_SIZE + 1; i++)
    {
        m_grid_x_unit_5[i] = new Prop(this);
        if (i == ((GRID_SIZE * 2 / GRID_UNIT_SIZE) / 2))
        {
            continue;
        }
        AddVertsForCube3D(m_grid_x_unit_5[i]->m_vertexes, Rgba8(191, 0, 0));
        m_grid_x_unit_5[i]->m_scale    = Vec3(GRID_SIZE * 2.f, 0.06f, 0.06f);
        m_grid_x_unit_5[i]->m_position = Vec3(0, GRID_SIZE * 1.f - static_cast<float>(i) * 5.f, 0);
    }

    m_grid_x_unit_1.resize(GRID_SIZE * 2 + 1);
    for (int i = 0; i < GRID_SIZE * 2 + 1; i++)
    {
        m_grid_x_unit_1[i] = new Prop(this);
        if ((i % GRID_UNIT_SIZE) == 0)
        {
            continue;
        }
        AddVertsForCube3D(m_grid_x_unit_1[i]->m_vertexes, Rgba8(127, 127, 127));
        m_grid_x_unit_1[i]->m_scale    = Vec3(GRID_SIZE * 2.f, 0.03f, 0.03f);
        m_grid_x_unit_1[i]->m_position = Vec3(0, GRID_SIZE * 1.f - static_cast<float>(i), 0);
    }

    m_grid_y_unit_5.resize(GRID_SIZE * 2 / GRID_UNIT_SIZE + 1);
    for (int i = 0; i < GRID_SIZE * 2 / GRID_UNIT_SIZE + 1; i++)
    {
        m_grid_y_unit_5[i] = new Prop(this);
        if (i == ((GRID_SIZE * 2 / GRID_UNIT_SIZE) / 2))
        {
            continue;
        }
        AddVertsForCube3D(m_grid_y_unit_5[i]->m_vertexes, Rgba8(0, 191, 0));
        m_grid_y_unit_5[i]->m_scale    = Vec3(0.06f, GRID_SIZE * 2.f, 0.06f);
        m_grid_y_unit_5[i]->m_position = Vec3(GRID_SIZE * 1.f - static_cast<float>(i) * 5.f, 0, 0);
    }

    m_grid_y_unit_1.resize(GRID_SIZE * 2 + 1);
    for (int i = 0; i < GRID_SIZE * 2 + 1; i++)
    {
        m_grid_y_unit_1[i] = new Prop(this);
        if ((i % GRID_UNIT_SIZE) == 0)
        {
            continue;
        }
        AddVertsForCube3D(m_grid_y_unit_1[i]->m_vertexes, Rgba8(127, 127, 127));
        m_grid_y_unit_1[i]->m_scale    = Vec3(0.03f, GRID_SIZE * 2.f, 0.03f);
        m_grid_y_unit_1[i]->m_position = Vec3(GRID_SIZE * 1.f - static_cast<float>(i), 0, 0);
    }
    ///

    /// Player controller
    m_player = new PlayerController(m_map, Vec3(2.5f, 8.5, 0.5f));

    /// Debug Drawing
#ifdef DEBUG_GRID
    // Arrows
    DebugAddWorldArrow(Vec3(1, 0, 0), Vec3(0, 0, 0), 0.12f, -1, Rgba8::RED, Rgba8::RED, DebugRenderMode::USE_DEPTH);
    DebugAddWorldArrow(Vec3(0, 1, 0), Vec3(0, 0, 0), 0.12f, -1, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::USE_DEPTH);
    DebugAddWorldArrow(Vec3(0, 0, 1), Vec3(0, 0, 0), 0.12f, -1, Rgba8::BLUE, Rgba8::GREEN, DebugRenderMode::USE_DEPTH);
    /// 

    // Text for y axis
    Mat44 transformY = Mat44::MakeTranslation3D(Vec3(0, 1.25f, 0.25f));
    transformY.AppendZRotation(180.f);
    DebugAddWorldText("y - left", transformY, 1.f, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::USE_DEPTH, Vec2(0.5, 0.5), -1);
    // Text for x axis
    Mat44 transformX = Mat44::MakeTranslation3D(Vec3(1.6f, 0, 0.25f));
    transformX.AppendZRotation(90.f);
    DebugAddWorldText("x - forward", transformX, 1.f, Rgba8::RED, Rgba8::RED, DebugRenderMode::USE_DEPTH, Vec2(0.5, 0.5), -1);
    // Text for z axis
    Mat44 transformZ = Mat44::MakeTranslation3D(Vec3(0, -0.25f, .9f));
    transformZ.AppendXRotation(-90.f);
    transformZ.AppendZRotation(180.f);
    DebugAddWorldText("z - up", transformZ, 1.f, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USE_DEPTH, Vec2(0.5, 0.5), -1);

#endif

    /// Game State
    g_theInput->SetCursorMode(CursorMode::POINTER);
}

Game::~Game()
{
    POINTER_SAFE_DELETE(m_grid_x)
    POINTER_SAFE_DELETE(m_grid_y)
    for (Prop* grid_x_unit_5 : m_grid_x_unit_5)
    {
        POINTER_SAFE_DELETE(grid_x_unit_5)
    }
    for (Prop* grid_x_unit_1 : m_grid_x_unit_1)
    {
        POINTER_SAFE_DELETE(grid_x_unit_1)
    }
    for (Prop* grid_y_unit_5 : m_grid_y_unit_5)
    {
        POINTER_SAFE_DELETE(grid_y_unit_5)
    }
    for (Prop* grid_y_unit_1 : m_grid_y_unit_1)
    {
        POINTER_SAFE_DELETE(grid_y_unit_1)
    }
    POINTER_SAFE_DELETE(m_ball)
    POINTER_SAFE_DELETE(m_cube_1)
    POINTER_SAFE_DELETE(m_cube)
    POINTER_SAFE_DELETE(m_map)
    POINTER_SAFE_DELETE(m_player)
    POINTER_SAFE_DELETE(m_testProp)
    POINTER_SAFE_DELETE(m_screenCamera)
    POINTER_SAFE_DELETE(m_worldCamera)
    POINTER_SAFE_DELETE(m_player)
    MapDefinition::ClearDefinitions();
}


void Game::Render() const
{
    g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
    g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
    g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);
    if (m_currentState == GameState::PLAYING)
    {
        g_theRenderer->BeingCamera(*m_player->m_camera);
        m_map->Render();
        g_theRenderer->EndCamera(*m_player->m_camera);
        g_theRenderer->BindShader(nullptr);
        /// Grid
        if (m_bEnableGrid)
            RenderGrids();
        /// Props
        if (m_bEnableTestObj)
            RenderProps();
        DebugRenderWorld(*g_theGame->m_player->m_camera);
        DebugRenderScreen(*g_theGame->m_screenCamera);
    }

    //======================================================================= End of World Render =======================================================================
    // Second render screen camera
    g_theRenderer->BeingCamera(*m_screenCamera);
    /// Display Only
#ifdef COSMIC
    if (m_currentState == GameState::ATTRACT)
    {
        g_theRenderer->ClearScreen(g_theApp->m_backgroundColor);
        g_theRenderer->BindTexture(nullptr);
        DebugDrawRing(Vec2(800, 400), m_currentIconCircleThickness, m_currentIconCircleThickness / 10, Rgba8::WHITE);
    }
#endif
    // UI render
    g_theRenderer->EndCamera(*m_screenCamera);
    //======================================================================= End of Screen Render =======================================================================
    /// 
}


void Game::UpdateCameras(float deltaTime)
{
    m_screenCamera->Update(deltaTime);
}

void Game::RenderGrids() const
{
    m_grid_x->Render();
    m_grid_y->Render();
    for (Prop* grid_x_unit : m_grid_x_unit_5)
    {
        grid_x_unit->Render();
    }
    for (Prop* grid_x_unit : m_grid_x_unit_1)
    {
        grid_x_unit->Render();
    }
    for (Prop* grid_y_unit : m_grid_y_unit_5)
    {
        grid_y_unit->Render();
    }
    for (Prop* grid_y_unit : m_grid_y_unit_1)
    {
        grid_y_unit->Render();
    }
}

void Game::RenderProps() const
{
    g_theRenderer->BindTexture(nullptr);
    m_cube->Render();
    m_cube_1->Render();
    m_ball->Render();
    //m_testProp->Render();
}


void Game::Update()
{
    if (m_currentState == GameState::ATTRACT)
    {
        g_theInput->SetCursorMode(CursorMode::POINTER);
    }

    /// PlayerController
    if (m_player)
    {
        m_player->Update(Clock::GetSystemClock().GetDeltaSeconds());
        DebugAddMessage(Stringf("PlayerController position: %.2f, %.2f, %.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z), 0);
    }
    ///

    /// Map
    if (m_map)
        m_map->Update();
    /// 

    /// Cube
    float brightnessFactor = CycleValue(m_clock->GetTotalSeconds(), 1.f);
    auto  color            = Rgba8(
        static_cast<unsigned char>(brightnessFactor * 255),
        static_cast<unsigned char>(brightnessFactor * 255),
        static_cast<unsigned char>(brightnessFactor * 255),
        255);
    m_cube_1->m_color = color;
    m_cube->m_orientation.m_rollDegrees += m_clock->GetDeltaSeconds() * 30;
    m_cube->m_orientation.m_pitchDegrees += m_clock->GetDeltaSeconds() * 30;
    /// 

    /// Sphere
    m_ball->m_orientation.m_yawDegrees += m_clock->GetDeltaSeconds() * 45;
    /// 

    /// Debug Only
    std::string debugGameState = Stringf("Time: %.2f FPS: %.1f Scale: %.2f",
                                         m_clock->GetTotalSeconds(),
                                         m_clock->GetFrameRate(),
                                         m_clock->GetTimeScale()
    );
    DebugAddScreenText(debugGameState, m_screenSpace, 14, 0);
    /// Display Only
#ifdef COSMIC
    m_counter++;
    m_currentIconCircleThickness = FluctuateValue(m_iconCircleRadius, 50.f, 0.02f, static_cast<float>(m_counter));
#endif
    float deltaTime = m_clock->GetDeltaSeconds();
    UpdateCameras(deltaTime);

    if (m_currentState == GameState::PLAYING)
    {
        HandleEntityCollisions();
    }

    HandleMouseEvent(deltaTime);
    HandleKeyBoardEvent(deltaTime);
}

void Game::EndFrame()
{
    if (m_map)
        m_map->EndFrame();
}

bool Game::GameStartEvent(EventArgs& args)
{
    UNUSED(args)
    printf("Event::GameStartEvent    Starting game...\n");
    Game* m_game = g_theGame;
    m_game->EnterState(GameState::PLAYING);
    g_theInput->SetCursorMode(CursorMode::FPS);

    std::string defaultMapName = g_gameConfigBlackboard.GetValue("defaultMap", "Default");
    m_game->m_map              = new Map(m_game, MapDefinition::GetByName(defaultMapName));
    return true;
}

bool Game::GameExitEvent(EventArgs& args)
{
    UNUSED(args)
    printf("Event::GameStartEvent    Exiting game...\n");
    Game* m_game = g_theGame;

    delete m_game->m_map;
    m_game->m_map = nullptr;

    m_game->EnterState(GameState::ATTRACT);
    g_theInput->SetCursorMode(CursorMode::POINTER);
    return true;
}

void Game::EnterState(GameState state)
{
    m_currentState = state;
}

void Game::ExitState(GameState state)
{
    UNUSED(state)
}


void Game::HandleKeyBoardEvent(float deltaTime)
{
    UNUSED(deltaTime)
    const XboxController& controller = g_theInput->GetController(0);
    if (m_currentState == GameState::ATTRACT)
    {
        bool spaceBarPressed = g_theInput->WasKeyJustPressed(32);
        bool NKeyPressed     = g_theInput->WasKeyJustPressed('N') || controller.WasButtonJustPressed(XBOX_BUTTON_A) || controller.WasButtonJustPressed(XBOX_BUTTON_START);
        if (spaceBarPressed || NKeyPressed)
        {
            g_theEventSystem->FireEvent("GameStartEvent");
        }
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
    {
        if (m_currentState == GameState::PLAYING)
        {
            g_theEventSystem->FireEvent("GameExitEvent");
        }
        else
        {
            g_theEventSystem->FireEvent("WindowCloseEvent");
        }
    }
}

void Game::HandleMouseEvent(float deltaTime)
{
    UNUSED(deltaTime)
}


void Game::RenderEntities() const
{
}

void Game::HandleEntityCollisions()
{
}
