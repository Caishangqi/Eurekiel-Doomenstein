#pragma once
#include <string>
#include <vector>

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"

class TileDefinition
{
    friend class Map;

public:
    static std::vector<TileDefinition> s_definitions;
    static void                        LoadDefinitions(const char* path);
    static void                        ClearDefinitions();
    static TileDefinition*             GetByName(const std::string& name);
    static TileDefinition*             GetByTexelColor(const Rgba8& color);

    TileDefinition(const XmlElement& tileDefElement);

    std::string m_name    = "Unknown";
    bool        m_isSolid = false;

protected:
    Rgba8   m_mapImagePixelColor  = Rgba8::WHITE;
    IntVec2 m_floorSpriteCoords   = IntVec2::INVALID;
    IntVec2 m_ceilingSpriteCoords = IntVec2::INVALID;
    IntVec2 m_wallSpriteCoords    = IntVec2::INVALID;
};
