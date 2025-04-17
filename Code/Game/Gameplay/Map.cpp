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
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/DebugRenderSystem.h"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Definition/ActorDefinition.hpp"
#include "Game/Framework/PlayerController.hpp"
#include "Game/Definition/TileDefinition.hpp"
#include "Engine/Renderer/Renderer.cpp"
#include "Game/Framework/ActorHandle.hpp"

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
    /*AddActorsToMap(new Actor(Vec3(7.5f, 8.5f, 0.25f), EulerAngles(), Rgba8::RED, 0.75f, 0.35f, true));
    AddActorsToMap(new Actor(Vec3(8.5f, 8.5f, 0.125f), EulerAngles(), Rgba8::RED, 0.75f, 0.35f, true));
    AddActorsToMap(new Actor(Vec3(9.5f, 8.5f, 0.0f), EulerAngles(), Rgba8::RED, 0.75f, 0.35f, true));*/

    //Actor* testActor = new Actor(Vec3(5.5f, 8.5f, 0.0f), EulerAngles(), Rgba8::BLUE, 0.125f, 0.0625, false);
    //AddActorsToMap(testActor);

    /// Add Actors
    for (SpawnInfo spawnInfo : m_definition->m_spawnInfos)
    {
        SpawnActor(spawnInfo);
    }
    
    for (PlayerController* controller : g_theGame->m_localPlayerControllers)
    {
        Actor* playerActor = SpawnPlayer(controller);
        controller->Possess(playerActor->m_handle);
    }
    ///
}

Map::~Map()
{
    printf("Map::Map    - Deleting Map from game \"%s\"\n", m_definition->m_name.c_str());
    m_texture = nullptr;

    m_shader = nullptr;

    delete m_indexBuffer;
    m_indexBuffer = nullptr;

    delete m_vertexBuffer;
    m_vertexBuffer = nullptr;

    for (Actor* actor : m_actors)
    {
        delete actor;
        actor = nullptr;
    }
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
    //m_vertexBuffer->Resize((int)m_vertexes.size() * sizeof(Vertex_PCUTBN));
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

IntVec2 Map::GetTileCoordsForWorldPos(const Vec3& worldCoords)
{
    return GetTileCoordsForWorldPos(Vec2(worldCoords.x, worldCoords.y));
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

bool Map::GetTileIsSolid(const IntVec2& coords)
{
    if (!GetTileIsInBound(coords))
    {
        return true;
    }
    return GetTile(coords)->GetTileDefinition()->m_isSolid;
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
        AABB2 space = m_game->m_screenSpace;
        space.m_maxs.y -= 30;
        DebugAddScreenText(Stringf("  Sun Direction X: %0.2f [F2 / F3 to change] ", m_sunDirection.x), space, 12, 0, Rgba8::WHITE, Rgba8::WHITE);
        space.m_maxs.y -= 14;
        DebugAddScreenText(Stringf("  Sun Direction Y:                          ", m_sunDirection.y), space, 12, 0, Rgba8::WHITE, Rgba8::WHITE);
        DebugAddScreenText(Stringf("                   %0.2f [F4 / F5 to change] ", m_sunDirection.y), space, 12, 0, Rgba8::WHITE, Rgba8::WHITE);
        space.m_maxs.y -= 14;
        DebugAddScreenText(Stringf("    Sun Intensity: %0.2f [F6 / F7 to change] ", m_sunIntensity), space, 12, 0, Rgba8::WHITE, Rgba8::WHITE);
        space.m_maxs.y -= 14;
        DebugAddScreenText(Stringf("Ambient Intensity: %0.2f [F8 / F9 to change] ", m_ambientIntensity), space, 12, 0, Rgba8::WHITE, Rgba8::WHITE);
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
    CheckAndRespawnPlayer();
}

void Map::EndFrame()
{
    DeleteDestroyedActors();
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

void Map::Render(PlayerController* toPlayer)
{
    g_theRenderer->SetModelConstants(Mat44(), Rgba8::WHITE);
    g_theRenderer->BindShader(m_shader);
    g_theRenderer->BindTexture(m_texture);
    g_theRenderer->SetLightConstants(m_sunDirection, m_sunIntensity, m_ambientIntensity);
    g_theRenderer->DrawIndexedVertexBuffer(m_vertexBuffer, m_indexBuffer, (int)m_indices.size());
    g_theRenderer->BindShader(nullptr);
    for (Actor* actor : m_actors)
    {
        if (actor && actor->m_handle.IsValid() && actor->m_definition->m_visible)
        {
            actor->Render(toPlayer);
        }
    }
}

LightingConstants Map::GetLightConstants()
{
    LightingConstants constant;
    constant.AmbientIntensity = m_ambientIntensity;
    constant.SunDirection     = m_sunDirection;
    constant.SunIntensity     = m_sunIntensity;
    return constant;
}

RaycastResult3D Map::RaycastAll(const Vec3& start, const Vec3& direction, float distance)
{
    std::vector<RaycastResult3D> results;
    std::vector<RaycastResult3D> resultImpact;
    resultImpact.reserve(4);
    results.reserve(4);
    RaycastResult3D result;
    results.push_back(RaycastWorldActors(start, direction, distance));
    results.push_back(RaycastWorldZ(start, direction, distance));
    results.push_back(RaycastWorldXY(start, direction, distance));
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

RaycastResult3D Map::RaycastAll(Actor* actor, ActorHandle& resultActorHit, const Vec3& start, const Vec3& direction, float distance)
{
    std::vector<RaycastResult3D> results;
    std::vector<RaycastResult3D> resultImpact;
    resultImpact.reserve(4);
    results.reserve(4);
    RaycastResult3D result;
    results.push_back(RaycastWorldActors(actor, resultActorHit, start, direction, distance));
    results.push_back(RaycastWorldZ(start, direction, distance));
    results.push_back(RaycastWorldXY(start, direction, distance));
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

RaycastResult3D Map::RaycastWorldXY(const Vec3& start, const Vec3& direction, float distance)
{
    RaycastResult3D result;
    result.m_rayStartPos  = start;
    result.m_rayFwdNormal = direction;
    result.m_rayMaxLength = distance;

    // Initialization Steps (once per Raycast)
    IntVec2 tileCoords = GetTileCoordsForWorldPos(start);
    if (GetTileIsSolid(tileCoords) && start.z < 1.0f)
    {
        result.m_didImpact    = true;
        result.m_impactPos    = start;
        result.m_impactDist   = 0.0f;
        result.m_impactNormal = -direction; // Arbitrary choice for impact normal
        return result;
    }

    float fwdDistPerXCrossing    = std::abs(1.0f / direction.x);
    int   tileStepDirectionX     = (direction.x < 0) ? -1 : 1;
    float xAtFirstXCrossing      = (tileStepDirectionX > 0) ? (std::floor(start.x) + 1.0f) : std::floor(start.x);
    float xDistToFirstXCrossing  = (xAtFirstXCrossing - start.x) * tileStepDirectionX;
    float fwdDistAtNextXCrossing = xDistToFirstXCrossing * fwdDistPerXCrossing;

    float fwdDistPerYCrossing    = std::abs(1.0f / direction.y);
    int   tileStepDirectionY     = (direction.y < 0) ? -1 : 1;
    float yAtFirstYCrossing      = (tileStepDirectionY > 0) ? (std::floor(start.y) + 1.0f) : std::floor(start.y);
    float yDistToFirstYCrossing  = (yAtFirstYCrossing - start.y) * tileStepDirectionY;
    float fwdDistAtNextYCrossing = yDistToFirstYCrossing * fwdDistPerYCrossing;

    float fwdDistPerZCrossing    = fabsf(1.0f / direction.z);
    int   tileStepDirectionZ     = (direction.z < 0) ? -1 : 1;
    float zAtFirstZCrossing      = (tileStepDirectionZ > 0) ? (floorf(start.z) + 1.0f) : floorf(start.z);
    float zDistToFirstZCrossing  = (zAtFirstZCrossing - start.z) * tileStepDirectionZ;
    float fwdDistAtNextZCrossing = zDistToFirstZCrossing * fwdDistPerZCrossing;

    // Main Raycast Loop (after Initialization)
    while (true)
    {
        if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing)
        {
            if (fwdDistAtNextXCrossing > distance)
            {
                break; // Ray missed
            }
            tileCoords.x += tileStepDirectionX;
            float impactZ = start.z + direction.z * fwdDistAtNextXCrossing; // Calculate Z direction based on X direction
            if (GetTileIsSolid(tileCoords) && impactZ >= 0.0f && impactZ <= 1.0f)
            {
                result.m_didImpact    = true;
                result.m_impactDist   = fwdDistAtNextXCrossing;
                result.m_impactPos    = start + direction * fwdDistAtNextXCrossing;
                result.m_impactNormal = Vec3(static_cast<float>(-tileStepDirectionX), 0.0f, 0.f);
                return result;
            }
            fwdDistAtNextXCrossing += fwdDistPerXCrossing;
            fwdDistAtNextZCrossing += fwdDistPerZCrossing; // Increase Z direction based on X direction
        }
        else
        {
            if (fwdDistAtNextYCrossing > distance)
            {
                break; // Ray missed
            }
            tileCoords.y += tileStepDirectionY;
            float impactZ = start.z + direction.z * fwdDistAtNextYCrossing;
            if (GetTileIsSolid(tileCoords) && (impactZ >= 0.0f && impactZ <= 1.0f))
            {
                result.m_didImpact    = true;
                result.m_impactDist   = fwdDistAtNextYCrossing;
                result.m_impactPos    = start + direction * fwdDistAtNextYCrossing;
                result.m_impactNormal = Vec3(0.0f, static_cast<float>(-tileStepDirectionY), 0.f);
                return result;
            }
            fwdDistAtNextYCrossing += fwdDistPerYCrossing;
            fwdDistAtNextZCrossing += fwdDistPerZCrossing;
        }
    }

    // No impact, ray missed
    result.m_impactDist = distance;
    result.m_impactPos  = start + direction * distance;
    return result;
}

RaycastResult3D Map::RaycastWorldZ(const Vec3& start, const Vec3& direction, float maxDistance)
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

    if (result.m_didImpact)
    {
        IntVec2 tileCoords = GetTileCoordsForWorldPos(Vec2(result.m_impactPos.x, result.m_impactPos.y));
        if (!GetTileIsInBound(tileCoords))
        {
            result.m_didImpact = false;
        }
    }

    return result;
}

RaycastResult3D Map::RaycastWorldActors(const Vec3& start, const Vec3& direction, float distance)
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

RaycastResult3D Map::RaycastWorldActors(Actor* actor, ActorHandle& resultActorHit, const Vec3& start, const Vec3& direction, float distance)
{
    struct ResultPair
    {
        RaycastResult3D raycastResult;
        ActorHandle     hitActor;

        ResultPair() = default;

        ResultPair(const RaycastResult3D& result, const ActorHandle& actor)
            : raycastResult(result), hitActor(actor)
        {
        }
    };

    ResultPair closestHit;
    float      closestDistance = FLT_MAX;
    bool       hasHit          = false;

    for (Actor* testActor : m_actors)
    {
        if (testActor == actor || !testActor)
            continue;
        if (testActor->m_definition->m_visible == false)
            continue;
        RaycastResult3D result = RaycastVsZCylinder3D(start, direction, distance, testActor->GetColliderZCylinder());
        if (result.m_didImpact && result.m_impactDist < closestDistance)
        {
            closestDistance = result.m_impactDist;
            closestHit      = ResultPair(result, testActor->m_handle);
            hasHit          = true;
        }
    }

    if (!hasHit)
    {
        RaycastResult3D missResult;
        missResult.m_rayStartPos  = start;
        missResult.m_rayFwdNormal = direction;
        missResult.m_rayMaxLength = distance;
        resultActorHit            = ActorHandle::INVALID;
        return missResult;
    }

    resultActorHit = closestHit.hitActor;
    return closestHit.raycastResult;
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

Actor* Map::AddActorsToMap(Actor* actor)
{
    if (!actor)
        return nullptr;
    unsigned int newIndex = 0;
    m_actors.push_back(nullptr);
    ++m_nextActorUID;
    ActorHandle handle(m_nextActorUID, newIndex);
    actor->m_map       = this;
    actor->m_handle    = handle;
    m_actors[newIndex] = actor;
    return actor;
}

Actor* Map::SpawnActor(const SpawnInfo& spawnInfo)
{
    unsigned int newIndex = 0;
    newIndex              = (unsigned int)m_actors.size();
    m_actors.push_back(nullptr);
    ++m_nextActorUID;
    ActorHandle handle(m_nextActorUID, newIndex);
    Actor*      actor  = new Actor(spawnInfo);
    actor->m_handle    = handle;
    actor->m_map       = this;
    m_actors[newIndex] = actor;
    actor->PostInitialize();
    return actor;
}

Actor* Map::SpawnPlayer(PlayerController* playerController)
{
    SpawnInfo spawnInfo;
    spawnInfo.m_actorName = "Marine";
    std::vector<Actor*> spawnPoints;
    GetActorsByName(spawnPoints, "SpawnPoint");
    int randomIndex = g_rng->RollRandomIntInRange(0, (int)spawnPoints.size() - 1);
    Actor* spawnPoint    = spawnPoints[randomIndex];
    spawnInfo.m_position = spawnPoint->m_position;
    //spawnPoint->m_orientation.m_yawDegrees = -90;
    spawnInfo.m_orientation = Vec3(spawnPoint->m_orientation);
    spawnInfo.m_velocity    = spawnPoint->m_velocity;
    Actor* playerActor      = SpawnActor(spawnInfo);
    playerController->m_map = this;
    return playerActor;
}

void Map::CheckAndRespawnPlayer()
{
    for (PlayerController* controller : g_theGame->m_localPlayerControllers)
    {
        if (!controller->GetActor())
        {
            Actor* playerActor = SpawnPlayer(controller);
            printf("Map::CheckAndRespawnPlayer      Player spawned at: (%f, %f, %f)\n", playerActor->m_position.x, playerActor->m_position.y, playerActor->m_position.z);
            controller->Possess(playerActor->m_handle);
        }
    }
}

Actor* Map::GetActorByHandle(const ActorHandle handle) const
{
    if (!handle.IsValid())
    {
        return nullptr;
    }
    unsigned int index = handle.GetIndex();
    if (index >= m_actors.size())
    {
        return nullptr;
    }
    Actor* actor = m_actors[index];
    if (actor == nullptr)
    {
        return nullptr;
    }
    if (actor->m_handle != handle)
    {
        return nullptr;
    }
    return actor;
}

Actor* Map::GetActorByName(const std::string& name) const
{
    for (Actor* actor : m_actors)
    {
        if (actor && actor->m_handle.IsValid())
        {
            if (actor->m_definition->m_name == name)
            {
                return actor;
            }
        }
    }
    return nullptr;
}

Actor* Map::GetClosestVisibleEnemy(Actor* instigator)
{
    float  closestDistSq = FLT_MAX;
    Actor* closestEnemy  = nullptr;

    for (Actor* actor : m_actors)
    {
        if (actor == nullptr || actor == instigator)
            continue;
        // Skip same faction or neutral
        if (actor->m_definition->m_faction == instigator->m_definition->m_faction)
        {
            continue;
        }
        if (actor->m_definition->m_faction == "NEUTRAL"
            || instigator->m_definition->m_faction == "NEUTRAL")
        {
            continue;
        }

        // Check distance
        Vec2 actorPos2D      = Vec2(actor->m_position.x, actor->m_position.y);
        Vec2 instigatorPos2D = Vec2(instigator->m_position.x, instigator->m_position.y);

        float distanceSq = GetDistanceSquared2D(actorPos2D, instigatorPos2D);
        float radiusSq   = instigator->m_definition->m_sightRadius * instigator->m_definition->m_sightRadius;
        if (distanceSq > radiusSq)
        {
            continue; // too far
        }

        // Check angle
        Vec3 fwd3, left3, up3;
        instigator->m_orientation.GetAsVectors_IFwd_JLeft_KUp(fwd3, left3, up3);

        Vec3 direction3D = actor->GetActorEyePosition() - instigator->GetActorEyePosition();

        Vec2 fwd2D(fwd3.x, fwd3.y);
        Vec2 dirToActor = (actorPos2D - instigatorPos2D).GetNormalized();

        // The angle between forward vector and direction to the actor
        float angleBetween = GetAngleDegreesBetweenVectors2D(fwd2D, dirToActor);
        if (angleBetween > instigator->m_definition->m_sightAngle * 0.5f)
        {
            continue; // out of FOV cone
        }
        /// Line of Sight check: make sure no walls blocking
        ActorHandle     resultActor;
        RaycastResult3D result = RaycastAll(instigator, resultActor, instigator->GetActorEyePosition(), direction3D.GetNormalized(), distanceSq);
        if (!result.m_didImpact) continue;
        //DebugAddWorldSphere(result.m_impactPos, 0.1f, 0);
        if (!IsPointInsideDisc2D(Vec2(result.m_impactPos.x, result.m_impactPos.y), Vec2(actor->m_position.x, actor->m_position.y), actor->m_physicalRadius + 0.1f))
        {
            continue;
        }
        /// End of Line of Sight check

        if (distanceSq < closestDistSq)
        {
            closestDistSq = distanceSq;
            closestEnemy  = actor;
        }
    }

    if (closestEnemy)
    {
        /*printf("Find Enemy: %s, Dist=%.2f\n",
               closestEnemy->m_definition->m_name.c_str(),
               sqrtf(closestDistSq));*/
    }
    return closestEnemy;
}


void Map::GetActorsByName(std::vector<Actor*>& inActors, const std::string& name) const
{
    for (Actor* actor : m_actors)
    {
        if (actor && actor->m_handle.IsValid())
        {
            if (actor->m_definition->m_name == name)
            {
                inActors.push_back(actor);
            }
        }
    }
}

Actor* Map::DebugPossessNext()
{
    if (!m_game->GetIsSingleMode())
        return nullptr;
    PlayerController* playerController = m_game->m_localPlayerControllers[0];
    if (!playerController)
        return nullptr;
    Actor* playerControlledActor = playerController->GetActor();
    if (playerControlledActor == nullptr)
    {
        for (Actor* actor : m_actors)
        {
            if (actor && actor->m_handle.IsValid() && actor->m_definition->m_canBePossessed)
            {
                playerController->Possess(actor->m_handle);
                return actor;
            }
        }
    }
    unsigned int index = playerControlledActor->m_handle.GetIndex();
    for (unsigned int i = index + 1; i <= m_actors.size() + index; i++)
    {
        unsigned int desiredIndex = i % (int)m_actors.size();

        if (m_actors[desiredIndex] && m_actors[desiredIndex]->m_handle.IsValid() && m_actors[desiredIndex]->m_definition->m_canBePossessed)
        {
            Actor* desiredControlledActor = m_actors[desiredIndex];
            playerController->Possess(desiredControlledActor->m_handle);
            return desiredControlledActor;
        }
    }
    return nullptr;
}

void Map::DeleteDestroyedActors()
{
    for (Actor* actor : m_actors)
    {
        if (actor && actor->m_handle.IsValid() && actor->m_bIsGarbage)
        {
            unsigned int index = actor->m_handle.GetIndex();
            delete actor;
            m_actors[index] = nullptr;
        }
    }
}
