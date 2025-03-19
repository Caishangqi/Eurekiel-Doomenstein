#include "MapDefinition.hpp"

#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"

/// Definitions
std::vector<MapDefinition> MapDefinition::s_definitions = {};
/// 

void MapDefinition::LoadDefinitions(const char* path)
{
    printf("MapDefinition::LoadDefinitions    %s", "Start Loading MapDefinitions\n");
    XmlDocument mapDefinitions;
    XmlResult   result = mapDefinitions.LoadFile(path);
    if (result == XmlResult::XML_SUCCESS)
    {
        XmlElement* rootElement = mapDefinitions.RootElement();
        if (rootElement)
        {
            printf("MapDefinition::LoadDefinitions    MapDefinitions config from file \"%s\" was loaded\n", path);
            XmlElement const* element = rootElement->FirstChildElement();
            while (element != nullptr)
            {
                MapDefinition mapDef = MapDefinition(*element);
                MapDefinition::s_definitions.push_back(mapDef);
                element = element->NextSiblingElement();
            }
        }
        else
        {
            printf("MapDefinition::LoadDefinitions    MapDefinitions config from file \"%s\"was invalid (missing root element)\n", path);
        }
    }
    else
    {
        printf("MapDefinition::LoadDefinitions    Failed to load MapDefinitions config from file \"%s\"\n", path);
    }
}

void MapDefinition::ClearDefinitions()
{
}

const MapDefinition* MapDefinition::GetByName(const std::string& name)
{
    for (int i = 0; i < (int)s_definitions.size(); i++)
    {
        if (s_definitions[i].m_name == name)
        {
            return &s_definitions[i];
        }
    }
    return nullptr;
}

MapDefinition::MapDefinition(XmlElement const& mapDefElement)
{
    m_name                 = ParseXmlAttribute(mapDefElement, "name", m_name);
    m_mapImage             = g_theRenderer->CreateImageFromFile(ParseXmlAttribute(mapDefElement, "image", m_name).c_str());
    m_spriteSheetCellCount = ParseXmlAttribute(mapDefElement, "spriteSheetCellCount", m_spriteSheetCellCount);
    m_spriteSheet          = new SpriteSheet(*g_theRenderer->CreateOrGetTextureFromFile(ParseXmlAttribute(mapDefElement, "spriteSheetTexture", m_name).c_str()), m_spriteSheetCellCount);
    m_shader               = g_theRenderer->CreateShaderFromFile(ParseXmlAttribute(mapDefElement, "shader", m_name).c_str(), VertexType::Vertex_PCUTBN);
    printf("MapDefinition::MapDefinition    — Create Definition \"%s\" \n", m_name.c_str());
    printf("                                ‖ Map SpriteSheet Dimension: %d x %d \n", m_spriteSheetCellCount.x, m_spriteSheetCellCount.y);
    printf("                                ‖ Map Dimension: %d x %d \n", m_mapImage->GetDimensions().x, m_mapImage->GetDimensions().y);
}
