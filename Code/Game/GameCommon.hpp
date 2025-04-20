#pragma once
#include <vector>

/// Whether or not enable cosmic circle (developer)
#define COSMIC
//#define DEBUG_GRID

#define PLAY_SOUND_CLICK(configKey) \
do { \
SoundID sid = g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue(configKey, "")); \
g_theAudio->StartSound(sid, false, 0.5f); \
} while (0)

#define IS_DEBUG_ENABLED() (g_gameConfigBlackboard.GetValue("enableDebug", false))

struct Vertex_PCU;
struct Rgba8;
struct Vec2;
class Camera;
class App;
class RandomNumberGenerator;
class Renderer;
class InputSystem;
class AudioSystem;
class Game;
class WidgetSubsystem;
class ResourceSubsystem;
class PlayerSaveSubsystem;

extern RandomNumberGenerator* g_rng;
extern App*                   g_theApp;
extern Renderer*              g_theRenderer;
extern InputSystem*           g_theInput;
extern AudioSystem*           g_theAudio;
extern Game*                  g_theGame;
extern WidgetSubsystem*       g_theWidgetSubsystem;
extern ResourceSubsystem*     g_theResourceSubsystem;
extern PlayerSaveSubsystem*   g_thePlayerSaveSubsystem;


/// Loaders

/// 

constexpr float WORLD_SIZE_X   = 200.f;
constexpr float WORLD_SIZE_Y   = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;
// Math
constexpr float PI = 3.14159265359f;
// Entity Data
constexpr int MAX_ENTITY_PER_TYPE = 64;

/// Grid
constexpr int GRID_SIZE      = 50; // Half
constexpr int GRID_UNIT_SIZE = 5;
/// 

/// Enums
enum class DeviceType
{
    KEYBOARD_AND_MOUSE = 0,
    CONTROLLER = 1,
};

inline const char* to_string(DeviceType e)
{
    switch (e)
    {
    case DeviceType::KEYBOARD_AND_MOUSE: return "Keyboard and Mouse";
    case DeviceType::CONTROLLER: return "Controller";
    default: return "unknown";
    }
}

void DebugDrawRing(const Vec2& center, float radius, float thickness, const Rgba8& color);

void DebugDrawLine(const Vec2& start, const Vec2& end, float thickness, const Rgba8& color);

void AddVertsForCube3D(std::vector<Vertex_PCU>& verts, const Rgba8& color);
void AddVertsForCube3D(std::vector<Vertex_PCU>& verts, const Rgba8& colorX, const Rgba8& colorNX, const Rgba8& colorY, const Rgba8& colorNY, const Rgba8& colorZ, const Rgba8& colorNZ);
