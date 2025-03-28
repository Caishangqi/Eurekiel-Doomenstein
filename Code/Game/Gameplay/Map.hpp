#pragma once
#include <vector>

#include "Actor.hpp"
#include "Tile.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Game/Definition/MapDefinition.hpp"

struct ActorHandle;
class VertexBuffer;
class Texture;
struct Vertex_PCUTBN;
class Game;
class MapDefinition;
class IndexBuffer;
class Shader;
struct RaycastResult3D;
class Actor;
class AABB2;
class AABB3;
struct Vertex_PCU;


class Map
{
    friend class PlayerController;

public:
    Map(Game* game, const MapDefinition* definition);
    ~Map();

    void CreateTiles();
    void CreateGeometry();
    void AddGeometryForWall(std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indices, const AABB3& bounds, const AABB2& UVs) const;
    void AddGeometryForFloor(std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indices, const AABB3& bounds, const AABB2& UVs) const;
    void AddGeometryForCeiling(std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indices, const AABB3& bounds, const AABB2& UVs) const;
    void CreateBuffers();

    bool    IsPositionInBounds(Vec3 position, const float tolerance = 0.f) const;
    IntVec2 GetTileCoordsForWorldPos(const Vec2& worldCoords);
    IntVec2 GetTileCoordsForWorldPos(const Vec3& worldCoords);
    bool    AreCoordsInBounds(int x, int y) const;
    bool    AreCoordsInBounds(IntVec2 coords) const;
    Tile*   GetTile(int x, int y);
    Tile*   GetTile(IntVec2 coords);
    Tile*   GetTile(const Vec2& worldCoords);
    bool    GetTileIsInBound(const IntVec2& coords);
    bool    GetTileIsSolid(const IntVec2& coords);

    void Update();
    void EndFrame();
    void ColliedWithActors();
    void ColliedActors(Actor* actorA, Actor* actorB);
    void ColliedActorsWithMap();
    void ColliedActorWithMap(Actor* actor);
    void PushActorOutOfTile(Actor* actor, const IntVec2& tileCoords);

    void Render();

    /// Raycast
    RaycastResult3D RaycastAll(const Vec3& start, const Vec3& direction, float distance);
    RaycastResult3D RaycastWorldXY(const Vec3& start, const Vec3& direction, float distance);
    RaycastResult3D RaycastWorldZ(const Vec3& start, const Vec3& direction, float distance);
    RaycastResult3D RaycastWorldActors(const Vec3& start, const Vec3& direction, float distance);

    ///
    /// Lighting
    void HandleDecreaseSunDirectionX();
    void HandleIncreaseSunDirectionX();
    void HandleDecreaseSunDirectionY();
    void HandleIncreaseSunDirectionY();
    void HandleDecreaseSunIntensity();
    void HandleIncreaseSunIntensity();
    void HandleDecreaseAmbientIntensity();
    void HandleIncreaseAmbientIntensity();
    /// 
    /// Actor
    Actor* AddActorsToMap(Actor* actor);
    Actor* SpawnActor(const SpawnInfo& spawnInfo);
    Actor* SpawnPlayer(PlayerController* playerController); // Spawn a marine actor at a random spawn point and possess it with the player.
    Actor* GetActorByHandle(const ActorHandle handle) const;
    Actor* GetActorByName(const std::string& name) const;
    Actor* GetClosestVisibleEnemy(); //Search the actor list to find actors meeting the provided criteria.
    void   GetActorsByName(std::vector<Actor*>& inActors, const std::string& name) const;
    Actor* DebugPossessNext(); // Have the player controller possess the next actor in the list that can be possessed
    void   DeleteDestroyedActors(); // Delete any actors marked as destroyed.

    /// 
    Game* m_game = nullptr;

protected:
    // Map
    const MapDefinition* m_definition = nullptr;
    std::vector<Tile>    m_tiles;
    IntVec2              m_dimensions;

    /// Actors
    std::vector<Actor*>       m_actors;
    static const unsigned int MAX_ACTOR_UID  = 0x0000fffeu;
    unsigned int              m_nextActorUID = 0;
    /// 

    /// Lighting
    Vec3  m_sunDirection     = Vec3(2, -1, -1);
    float m_sunIntensity     = 0.85f;
    float m_ambientIntensity = 0.35f;
    /// 

    // Rendering
    std::vector<Vertex_PCUTBN> m_vertexes;
    std::vector<unsigned int>  m_indices;
    Texture*                   m_texture      = nullptr;
    Shader*                    m_shader       = nullptr;
    VertexBuffer*              m_vertexBuffer = nullptr;
    IndexBuffer*               m_indexBuffer  = nullptr;
};
