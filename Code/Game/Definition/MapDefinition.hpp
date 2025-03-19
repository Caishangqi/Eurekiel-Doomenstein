#pragma once
#include <string>
#include <vector>

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"

class Texture;
class Shader;
class SpriteSheet;
class Image;

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
    std::string  m_name                 = "Default";
    Image*       m_mapImage             = nullptr;
    SpriteSheet* m_spriteSheet          = nullptr;
    Shader*      m_shader               = nullptr;
    IntVec2      m_spriteSheetCellCount = IntVec2::ZERO;
};
