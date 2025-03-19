#include "Map.hpp"

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.h"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"
#include "Game/Definition/TileDefinition.hpp"

Map::Map(Game* game, const MapDefinition* definition): m_game(game), m_definition(definition)
{
    printf("Map::Map    + Creating Map from the definition \"%s\"\n", definition->m_name.c_str());
    m_texture    = g_theRenderer->CreateTextureFromFile("Data/Images/Terrain_8x8.png");
    m_dimensions = definition->m_mapImage->GetDimensions();
    m_shader     = definition->m_shader;
    CreateTiles();
    CreateGeometry();
    CreateBuffers();

    /// Testing Adding Actors
    AddActorsToMap(new Actor(Vec3(7.5f, 8.5f, 0.25f), EulerAngles(), Rgba8::RED, 0.75f, 0.35f, true));
    AddActorsToMap(new Actor(Vec3(8.5f, 8.5f, 0.125f), EulerAngles(), Rgba8::RED, 0.75f, 0.35f, true));
    AddActorsToMap(new Actor(Vec3(9.5f, 8.5f, 0.0f), EulerAngles(), Rgba8::RED, 0.75f, 0.35f, true));

    Actor* testActor = new Actor(Vec3(5.5f, 8.5f, 0.0f), EulerAngles(), Rgba8::BLUE, 0.125f, 0.0625, false);
    AddActorsToMap(testActor);
    m_game->m_player->BindActorToPlayer(testActor);
    /// 
}

Map::~Map()
{
    printf("Map::Map    - Deleting Map from game \"%s\"\n", m_definition->m_name.c_str());
    m_texture = nullptr;
    m_shader  = nullptr;

    delete m_indexBuffer;
    m_indexBuffer = nullptr;

    delete m_vertexBuffer;
    m_vertexBuffer = nullptr;
}

void Map::CreateTiles()
{
    printf("Map::Create       ‖ Creating Map tiles \n");
    m_tiles.resize((int)m_definition->m_mapImage->GetDimensions().x * m_definition->m_mapImage->GetDimensions().y);
    for (int y = 0; y < m_definition->m_mapImage->GetDimensions().y; y++)
    {
        for (int x = 0; x < m_definition->m_mapImage->GetDimensions().x; x++)
        {
            Tile* tile = GetTile(x, y);
            tile->SetTileCoords(IntVec2(x, y));
            tile->SetBounds(AABB3(Vec3((float)x, (float)y, 0.f), Vec3((float)(x + 1), (float)(y + 1), 1.f)));
            Rgba8           color      = m_definition->m_mapImage->GetTexelColor(IntVec2(x, y));
            TileDefinition* definition = TileDefinition::GetByTexelColor(color);
            if (definition == NULL)
                printf("Map::Create       ‖ Tile definition not found for texel color (%d, %d)", x, y);
            tile->SetTileDefinition(definition);
            //printf("Map::Create       ‖ Add tile %s at (%d, %d)\n", definition->m_name.c_str(), x, y);
        }
    }
    printf("Map::Create       ‖ Creating total tiles: %d\n", (int)m_tiles.size());
}

void Map::CreateGeometry()
{
    printf("Map::Create       ‖ Creating Map Geometry \n");
    for (Tile& tile : m_tiles)
    {
        TileDefinition* definition = tile.GetTileDefinition();
        if (!definition)
            continue;
        if (definition->m_floorSpriteCoords != IntVec2::INVALID)
        {
            AddGeometryForFloor(m_vertexes, m_indices, tile.GetBounds(), m_definition->m_spriteSheet->GetSpriteUVs(definition->m_floorSpriteCoords));
        }
        if (definition->m_wallSpriteCoords != IntVec2::INVALID)
        {
            AddGeometryForWall(m_vertexes, m_indices, tile.GetBounds(), m_definition->m_spriteSheet->GetSpriteUVs(definition->m_wallSpriteCoords));
        }
        if (definition->m_ceilingSpriteCoords != IntVec2::INVALID)
        {
            AddGeometryForCeiling(m_vertexes, m_indices, tile.GetBounds(), m_definition->m_spriteSheet->GetSpriteUVs(definition->m_ceilingSpriteCoords));
        }
    }
}

void Map::AddGeometryForWall(std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indices, const AABB3& bounds, const AABB2& UVs) const
{
    // -x
    AddVertsForQuad3D(vertexes, indices, Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),
                      bounds.m_mins,
                      Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),
                      Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z)
                      , Rgba8::WHITE, UVs);
    // +x
    AddVertsForQuad3D(vertexes, indices,
                      Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),
                      Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),
                      bounds.m_maxs,
                      Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z)
                      , Rgba8::WHITE, UVs);
    // -y
    AddVertsForQuad3D(vertexes, indices,
                      Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z),
                      Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z),
                      Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),
                      bounds.m_maxs
                      , Rgba8::WHITE, UVs);
    // +y
    AddVertsForQuad3D(vertexes, indices, bounds.m_mins,
                      Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z),
                      Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),
                      Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),
                      Rgba8::WHITE, UVs);
}

void Map::AddGeometryForFloor(std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indices, const AABB3& bounds, const AABB2& UVs) const
{
    Vec3 bottomLeft  = bounds.m_mins;
    Vec3 bottomRight = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
    Vec3 topRight    = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
    Vec3 topLeft     = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
    AddVertsForQuad3D(vertexes, indices, bottomLeft, bottomRight, topRight, topLeft, Rgba8::WHITE, UVs);
}

void Map::AddGeometryForCeiling(std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indices, const AABB3& bounds, const AABB2& UVs) const
{
    // +z
    AddVertsForQuad3D(vertexes, indices, bounds.m_maxs, Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z),
                      Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z),
                      Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z),
                      Rgba8::WHITE, UVs);
}

void Map::CreateBuffers()
{
    printf("Map::Create       ‖ Creating Buffers for optimization\n");
    printf("Map::Create       ‖ Creating Vertex Buffers...\n");
    /// TODO: Consider refactory those steps
    m_vertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN));
    m_vertexBuffer->Resize((int)m_vertexes.size() * sizeof(Vertex_PCUTBN));
    g_theRenderer->CopyCPUToGPU(m_vertexes.data(), (int)m_vertexes.size() * sizeof(Vertex_PCUTBN), m_vertexBuffer);
    printf("Map::Create       ‖ Creating Index Buffers...\n");
    m_indexBuffer = g_theRenderer->CreateIndexBuffer(sizeof(unsigned int));
    m_indexBuffer->Resize((int)m_indices.size() * sizeof(unsigned int));
    g_theRenderer->CopyCPUToGPU(m_indices.data(), (int)m_indices.size() * sizeof(unsigned int), m_indexBuffer);
}

bool Map::IsPositionInBounds(Vec3 position, const float tolerance) const
{
    UNUSED(position)
    UNUSED(tolerance)
    return true;
}

IntVec2 Map::GetTileCoordsForWorldPos(const Vec2& worldCoords)
{
    return IntVec2(static_cast<int>(floorf(worldCoords.x)), static_cast<int>(floorf(worldCoords.y)));
}

bool Map::AreCoordsInBounds(int x, int y) const
{
    UNUSED(x)
    UNUSED(y)
    return true;
}

bool Map::AreCoordsInBounds(IntVec2 coords) const
{
    UNUSED(coords)
    return true;
}

Tile* Map::GetTile(int x, int y)
{
    return &m_tiles[x + y * m_dimensions.x];
}


Tile* Map::GetTile(IntVec2 coords)
{
    return GetTile(coords.x, coords.y);
}

Tile* Map::GetTile(const Vec2& worldCoords)
{
    return GetTile(GetTileCoordsForWorldPos(worldCoords));
}

bool Map::GetTileIsInBound(const IntVec2& coords)
{
    return coords.x >= 0 && coords.y >= 0 && coords.x < m_dimensions.x && coords.y < m_dimensions.y;
}

void Map::Update()
{
    /// Lighting
    {
        HandleDecreaseSunDirectionX();
        HandleIncreaseSunDirectionX();
        HandleDecreaseSunDirectionY();
        HandleIncreaseSunDirectionY();
        HandleDecreaseSunIntensity();
        HandleIncreaseSunIntensity();
        HandleDecreaseAmbientIntensity();
        HandleIncreaseAmbientIntensity();
    }
    ///

    /// Actor
    {
        for (Actor* actor : m_actors)
        {
            if (actor)
            {
                actor->Update(g_theGame->m_clock->GetDeltaSeconds());
            }
        }
    }
    /// 
    ColliedWithActors();
    ColliedActorsWithMap();
}

void Map::ColliedWithActors()
{
    for (Actor* actor : m_actors)
    {
        if (actor)
        {
            for (Actor* otherActor : m_actors)
            {
                if (otherActor && actor != otherActor)
                {
                    ColliedActors(actor, otherActor);
                }
            }
        }
    }
}

void Map::ColliedActors(Actor* actorA, Actor* actorB)
{
    if (DoZCylinder3DOverlap(actorA->GetColliderZCylinder(), actorB->GetColliderZCylinder()))
    {
        actorA->OnColliedEnter(actorB);
        actorB->OnColliedEnter(actorA);
    }
}

void Map::ColliedActorsWithMap()
{
    for (Actor* actor : m_actors)
    {
        if (actor)
            ColliedActorWithMap(actor);
    }
}

void Map::ColliedActorWithMap(Actor* actor)
{
    IntVec2 tileCoords = GetTileCoordsForWorldPos(Vec2(actor->m_position.x, actor->m_position.y));
    // Push out of cardinal neighbors (N-S-E-W) first
    PushActorOutOfTile(actor, tileCoords + IntVec2(1, 0));
    PushActorOutOfTile(actor, tileCoords + IntVec2(0, 1));
    PushActorOutOfTile(actor, tileCoords + IntVec2(-1, 0));
    PushActorOutOfTile(actor, tileCoords + IntVec2(0, -1));

    // Push out diagonal neighbours seconds
    PushActorOutOfTile(actor, tileCoords + IntVec2(1, 1));
    PushActorOutOfTile(actor, tileCoords + IntVec2(-1, 1));
    PushActorOutOfTile(actor, tileCoords + IntVec2(-1, -1));
    PushActorOutOfTile(actor, tileCoords + IntVec2(1, -1));

    actor->OnColliedEnter(GetTile(tileCoords)->GetBounds());
}

void Map::PushActorOutOfTile(Actor* actor, const IntVec2& tileCoords)
{
    if (GetTileIsInBound(tileCoords))
    {
        if (GetTile(tileCoords)->IsTileSolid())
        {
            AABB2 aabb2;
            aabb2.m_mins = Vec2(static_cast<float>(tileCoords.x), static_cast<float>(tileCoords.y));
            aabb2.m_maxs = aabb2.m_mins + Vec2::ONE;
            actor->OnColliedEnter(aabb2);
        }
    }
}

void Map::Render()
{
    g_theRenderer->SetModelConstants(Mat44(), Rgba8::WHITE);
    g_theRenderer->BindShader(m_shader);
    g_theRenderer->BindTexture(m_texture);
    g_theRenderer->SetLightConstants(m_sunDirection.GetNormalized(), m_sunIntensity, m_ambientIntensity);
    g_theRenderer->DrawIndexedVertexBuffer(m_vertexBuffer, m_indexBuffer, (int)m_indices.size());
    g_theRenderer->BindShader(nullptr);
    for (Actor* actor : m_actors)
    {
        actor->Render();
    }
}

RaycastResult3D Map::RaycastAll(const Vec3& start, const Vec3& direction, float distance) const
{
    std::vector<RaycastResult3D> results;
    std::vector<RaycastResult3D> resultImpact;
    resultImpact.reserve(4);
    results.reserve(4);
    RaycastResult3D result;
    results.push_back(RaycastWorldActors(start, direction, distance));
    results.push_back(RaycastWorldZ(start, direction, distance));
    for (RaycastResult3D result_3d : results)
    {
        if (result_3d.m_didImpact)
        {
            resultImpact.push_back(result_3d);
        }
    }

    float tempDist = FLT_MAX;
    for (RaycastResult3D result_Dist : resultImpact)
    {
        if (result_Dist.m_impactDist < tempDist)
        {
            tempDist = result_Dist.m_impactDist;
            result   = result_Dist;
        }
    }

    return result;
}

RaycastResult3D Map::RaycastWorldXY(const Vec3& start, const Vec3& direction, float distance) const
{
    UNUSED(start)
    UNUSED(direction)
    UNUSED(distance)
    return RaycastResult3D();
}

RaycastResult3D Map::RaycastWorldZ(const Vec3& start, const Vec3& direction, float maxDistance) const
{
    RaycastResult3D result;
    result.m_rayFwdNormal = direction;
    result.m_rayMaxLength = maxDistance;
    result.m_rayStartPos  = start;

    if (fabs(direction.z) < 1e-6f)
    {
        result.m_didImpact = false;
        return result;
    }

    if (direction.z > 0.0f)
    {
        const float floorZ = 1.0f;
        float       t      = (floorZ - start.z) / direction.z;
        if (t >= 0.0f && t <= maxDistance)
        {
            result.m_didImpact    = true;
            result.m_impactDist   = t;
            result.m_impactPos    = start + direction * t;
            result.m_impactNormal = Vec3(0.0f, 0.0f, -1.0f);
        }
    }
    else
    {
        const float ceilingZ = 0.0f;
        float       t        = (ceilingZ - start.z) / direction.z;
        if (t >= 0.0f && t <= maxDistance)
        {
            result.m_didImpact    = true;
            result.m_impactDist   = t;
            result.m_impactPos    = start + direction * t;
            result.m_impactNormal = Vec3(0.0f, 0.0f, 1.0f);
        }
    }

    return result;
}

RaycastResult3D Map::RaycastWorldActors(const Vec3& start, const Vec3& direction, float distance) const
{
    RaycastResult3D result;

    for (Actor* actor : m_actors)
    {
        RaycastResult3D resultIndividual = RaycastVsZCylinder3D(start, direction, distance, actor->GetColliderZCylinder());
        if (resultIndividual.m_didImpact)
        {
            result = resultIndividual;
            break;
        }
    }
    return result;
}

void Map::RenderRaycastResult(const RaycastResult3D& result)
{
}

void Map::HandleDecreaseSunDirectionX()
{
    if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
    {
        m_sunDirection.x -= 1;
        DebugAddMessage(Stringf("Sun Direction x: %f", m_sunDirection.x), 5.f);
    }
}

void Map::HandleIncreaseSunDirectionX()
{
    if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
    {
        m_sunDirection.x += 1;
        DebugAddMessage(Stringf("Sun Direction x: %f", m_sunDirection.x), 5.f);
    }
}

void Map::HandleDecreaseSunDirectionY()
{
    if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
    {
        m_sunDirection.y -= 1;
        DebugAddMessage(Stringf("Sun Direction y: %f", m_sunDirection.y), 5.f);
    }
}

void Map::HandleIncreaseSunDirectionY()
{
    if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
    {
        m_sunDirection.y += 1;
        DebugAddMessage(Stringf("Sun Direction y: %f", m_sunDirection.y), 5.f);
    }
}

void Map::HandleDecreaseSunIntensity()
{
    if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
    {
        m_sunIntensity -= 0.05f;
        DebugAddMessage(Stringf("Sun Intensity: %f", m_sunIntensity), 5.f);
    }
}

void Map::HandleIncreaseSunIntensity()
{
    if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
    {
        m_sunIntensity += 0.05f;
        DebugAddMessage(Stringf("Sun Intensity: %f", m_sunIntensity), 5.f);
    }
}

void Map::HandleDecreaseAmbientIntensity()
{
    if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
    {
        m_ambientIntensity -= 0.05f;
        DebugAddMessage(Stringf("Ambient Intensity: %f", m_ambientIntensity), 5.f);
    }
}

void Map::HandleIncreaseAmbientIntensity()
{
    if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
    {
        m_ambientIntensity += 0.05f;
        DebugAddMessage(Stringf("Ambient Intensity: %f", m_ambientIntensity), 5.f);
    }
}

bool Map::AddActorsToMap(Actor* actor)
{
    if (actor)
    {
        m_actors.push_back(actor);
        return true;
    }
    return false;
}
