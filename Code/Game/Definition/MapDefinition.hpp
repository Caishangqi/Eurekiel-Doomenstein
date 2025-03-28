#pragma once
#include <string>
#include <vector>

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

class Texture;
class Shader;
class SpriteSheet;
class Image;

/// Map definitions have a new SpawnInfo child element.When a map is created,
/// it spawns actors at for each spawn info. This class, in addition to being
/// part of the map definition file, can also be used as an argument to functions
/// that spawning actors in general.
struct SpawnInfo
{
public:
    SpawnInfo() = default;

public:
    std::string m_actorName = "Default";
    std::string m_faction   = "NEUTRAL";
    Vec3        m_position;
    Vec3        m_orientation;
    Vec3        m_velocity;
};

class MapDefinition
{
public:
    static std::vector<MapDefinition> s_definitions;
    static void                       LoadDefinitions(const char* path);
    static void                       ClearDefinitions();
    static const MapDefinition*       GetByName(const std::string& name);

public:
    MapDefinition(XmlElement const& mapDefElement);

public:
    std::string            m_name                 = "Default";
    Image*                 m_mapImage             = nullptr;
    SpriteSheet*           m_spriteSheet          = nullptr;
    Shader*                m_shader               = nullptr;
    IntVec2                m_spriteSheetCellCount = IntVec2::ZERO;
    std::vector<SpawnInfo> m_spawnInfos;
};
