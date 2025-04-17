#pragma once
#include "GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/AABB2.hpp"

class Map;
class PlayerController;
class Clock;
class Prop;

enum class GameState
{
    NONE,
    ATTRACT,
    LOBBY,
    PLAYING,
    COUNT
};

class Game
{
public:
    Game();
    ~Game();
    void Render() const;
    void Update();
    void EndFrame();

    /// Event Handle
    STATIC bool GameExitEvent(EventArgs& args);
    STATIC bool GameStateChangeEvent(EventArgs& args);
    /// 

    /// Controller and Multiplayer
    PlayerController* CreateLocalPlayer(int id, DeviceType deviceType); // Create local player by unique id and expect device type, return null if id is identical.
    void              RemoveLocalPlayer(int id); // Remove local player by its unique id and automatically shrink the controller container.
    PlayerController* GetLocalPlayer(int id); // Return the PlayerController with specific controller id.
    PlayerController* GetControllerByDeviceType(DeviceType deviceType); // Return the first found controller that has the specific device type.
    bool              GetIsSingleMode() const;

    /// Game State
    void EnterState(GameState state);
    void ExitState(GameState state);
    void EnterAttractState();
    void EnterLobbyState();
    void EnterPlayingState();
    /// 

    void HandleKeyBoardEvent(float deltaTime);
    void HandleMouseEvent(float deltaTime);

    // Camera
    void UpdateCameras(float deltaTime);


public: /// Game State
    GameState m_currentState = GameState::ATTRACT;
    GameState m_nextState    = GameState::ATTRACT;
    ///

    /// Map
    Map* m_map = nullptr;
    /// 

    // Camera
    Camera* m_worldCamera  = nullptr;
    Camera* m_screenCamera = nullptr;

    // Space for both world and screen, camera needs them
    AABB2 m_screenSpace; // TODO: screen space need recalculate based on num of player.
    AABB2 m_worldSpace;

    /// Clock
    Clock* m_clock = nullptr;
    /// 

    /// PlayerController
    std::vector<PlayerController*> m_localPlayerControllers;
    /// 
};
