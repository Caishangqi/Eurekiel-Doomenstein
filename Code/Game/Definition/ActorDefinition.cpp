#include "ActorDefinition.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"

std::vector<ActorDefinition> ActorDefinition::s_definitions = {};

void ActorDefinition::LoadDefinitions(const char* path)
{
    printf("ActorDefinition::LoadDefinitions    %s", "Start Loading ActorDefinition\n");
    XmlDocument mapDefinitions;
    XmlResult   result = mapDefinitions.LoadFile(path);
    if (result == XmlResult::XML_SUCCESS)
    {
        XmlElement* rootElement = mapDefinitions.RootElement();
        if (rootElement)
        {
            printf("ActorDefinition::LoadDefinitions    ActorDefinitions from \"%s\" was loaded\n", path);
            XmlElement const* element = rootElement->FirstChildElement();
            while (element != nullptr)
            {
                ActorDefinition mapDef = ActorDefinition(*element);
                ActorDefinition::s_definitions.push_back(mapDef);
                element = element->NextSiblingElement();
            }
        }
        else
        {
            printf("ActorDefinition::LoadDefinitions    ActorDefinitions from \"%s\"was invalid (missing root element)\n", path);
        }
    }
    else
    {
        printf("ActorDefinition::LoadDefinitions    Failed to load ActorDefinitions from \"%s\"\n", path);
    }
}

void ActorDefinition::ClearDefinitions()
{
    for (ActorDefinition definition : s_definitions)
    {
        POINTER_SAFE_DELETE(definition.m_shader)
    }
}

ActorDefinition* ActorDefinition::GetByName(const std::string& name)
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

ActorDefinition::ActorDefinition(XmlElement const& actorDefElement)
{
    m_name                             = ParseXmlAttribute(actorDefElement, "name", m_name);
    m_faction                          = ParseXmlAttribute(actorDefElement, "faction", m_faction);
    m_health                           = ParseXmlAttribute(actorDefElement, "health", m_health);
    m_canBePossessed                   = ParseXmlAttribute(actorDefElement, "canBePossessed", m_canBePossessed);
    m_corpseLifetime                   = ParseXmlAttribute(actorDefElement, "corpseLifetime", m_corpseLifetime);
    m_visible                          = ParseXmlAttribute(actorDefElement, "visible", m_visible);
    m_dieOnSpawn                       = ParseXmlAttribute(actorDefElement, "dieOnSpawn", m_dieOnSpawn);
    const XmlElement* collisionElement = FindChildElementByName(actorDefElement, "Collision");
    if (collisionElement)
    {
        printf("                                    ‖ Loading Collision Information\n");
        m_physicsRadius      = ParseXmlAttribute(*collisionElement, "radius", m_physicsRadius);
        m_physicsHeight      = ParseXmlAttribute(*collisionElement, "height", m_physicsHeight);
        m_collidesWithWorld  = ParseXmlAttribute(*collisionElement, "collidesWithWorld", m_collidesWithWorld);
        m_collidesWithActors = ParseXmlAttribute(*collisionElement, "collidesWithActors", m_collidesWithActors);
        m_dieOnCollide       = ParseXmlAttribute(*collisionElement, "dieOnCollide", m_dieOnCollide);
        m_damageOnCollide    = ParseXmlAttribute(*collisionElement, "damageOnCollide", m_damageOnCollide);
        m_impulseOnCollied   = ParseXmlAttribute(*collisionElement, "impulseOnCollide", m_impulseOnCollied);
    }
    const XmlElement* physicsElement = FindChildElementByName(actorDefElement, "Physics");
    if (physicsElement)
    {
        printf("                                    ‖ Loading Physics Information\n");
        m_simulated = ParseXmlAttribute(*physicsElement, "simulated", m_simulated);
        m_flying    = ParseXmlAttribute(*physicsElement, "flying", m_flying);
        m_walkSpeed = ParseXmlAttribute(*physicsElement, "walkSpeed", m_walkSpeed);
        m_runSpeed  = ParseXmlAttribute(*physicsElement, "runSpeed", m_runSpeed);
        m_turnSpeed = ParseXmlAttribute(*physicsElement, "turnSpeed", m_turnSpeed);
        m_drag      = ParseXmlAttribute(*physicsElement, "drag", m_drag);
    }
    const XmlElement* cameraElement = FindChildElementByName(actorDefElement, "Camera");
    if (cameraElement)
    {
        printf("                                    ‖ Loading Camera Information\n");
        m_eyeHeight = ParseXmlAttribute(*cameraElement, "eyeHeight", m_eyeHeight);
        m_cameraFOV = ParseXmlAttribute(*cameraElement, "cameraFOV", m_cameraFOV);
    }
    const XmlElement* aiElement = FindChildElementByName(actorDefElement, "AI");
    if (aiElement)
    {
        printf("                                    ‖ Loading AI Information\n");
        m_aiEnabled   = ParseXmlAttribute(*aiElement, "aiEnabled", m_aiEnabled);
        m_sightRadius = ParseXmlAttribute(*aiElement, "sightRadius", m_sightRadius);
        m_sightAngle  = ParseXmlAttribute(*aiElement, "sightAngle", m_sightAngle);
    }
    const XmlElement* visualsElement = FindChildElementByName(actorDefElement, "Visuals");
    if (visualsElement)
    {
        printf("                                    ‖ Loading Visuals Information\n");
        m_cellCount     = ParseXmlAttribute(*visualsElement, "cellCount", m_cellCount);
        m_size          = ParseXmlAttribute(*visualsElement, "size", m_size);
        m_pivot         = ParseXmlAttribute(*visualsElement, "pivot", m_pivot);
        m_billboardType = ParseXmlAttribute(*visualsElement, "billboardType", m_billboardType);
        m_renderLit     = ParseXmlAttribute(*visualsElement, "renderLit", m_renderLit);
        m_renderRounded = ParseXmlAttribute(*visualsElement, "renderRounded", m_renderRounded);
        m_shader        = g_theRenderer->CreateShaderFromFile(ParseXmlAttribute(*visualsElement, "shader", m_name).c_str(), VertexType::Vertex_PCUTBN);
        m_spriteSheet   = new SpriteSheet(*g_theRenderer->CreateOrGetTextureFromFile(ParseXmlAttribute(*visualsElement, "spriteSheet", m_name).c_str()), m_cellCount);
        if (visualsElement->ChildElementCount() > 0)
        {
            /// Handle Animation
            XmlElement const* element = visualsElement->FirstChildElement();
            while (element != nullptr)
            {
                AnimationGroup animation_group = AnimationGroup(*element, *m_spriteSheet);
                m_animationGroups.push_back(animation_group);
                printf("                                 — Add AnimationGroup: %s\n", animation_group.m_name.c_str());
                element = element->NextSiblingElement();
            }
        }
    }
    /// Handle Sounds Loading
    const XmlElement* soundsElement = FindChildElementByName(actorDefElement, "Sounds");
    if (soundsElement)
    {
        printf("                                    ‖ Loading Sound Information\n");
        XmlElement const* element = soundsElement->FirstChildElement();
        while (element != nullptr)
        {
            Sound sound = Sound(*element);
            m_sounds.push_back(sound);
            printf("ActorDefinition::ActorDefinition    — Add Sound: %s From: %s\n", sound.m_name.c_str(), sound.m_filePath.c_str());
            element = element->NextSiblingElement();
        }
    }
    /// Handle Inventory Loading
    const XmlElement* inventoryElement = FindChildElementByName(actorDefElement, "Inventory");
    if (inventoryElement)
    {
        printf("                                    ‖ Loading Inventory Information\n");
        const XmlElement* weapon = inventoryElement->FirstChildElement();
        while (weapon != nullptr)
        {
            std::string weaponName = ParseXmlAttribute(*weapon, "name", weaponName);
            m_inventory.push_back(weaponName);
            weapon = weapon->NextSiblingElement();
        }
    }
    printf("ActorDefinition::ActorDefinition    — Create Definition \"%s\" \n", m_name.c_str());
}

AnimationGroup* ActorDefinition::GetAnimationGroupByName(std::string& name)
{
    for (AnimationGroup& animGroup : m_animationGroups)
    {
        if (animGroup.m_name == name)
            return &animGroup;
    }
    return nullptr;
}

Sound* ActorDefinition::GetSoundByName(std::string& name)
{
    for (Sound& sound : m_sounds)
    {
        if (sound.m_name == name)
            return &sound;
    }
    return nullptr;
}
