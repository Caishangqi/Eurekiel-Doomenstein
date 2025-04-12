#include "WeaponDefinition.hpp"
std::vector<WeaponDefinition> WeaponDefinition::s_definitions = {};

void WeaponDefinition::LoadDefinitions(const char* path)
{
    printf("MapDefinition::LoadDefinitions    %s", "Start Loading WeaponDefinition\n");
    XmlDocument mapDefinitions;
    XmlResult   result = mapDefinitions.LoadFile(path);
    if (result == XmlResult::XML_SUCCESS)
    {
        XmlElement* rootElement = mapDefinitions.RootElement();
        if (rootElement)
        {
            printf("WeaponDefinition::LoadDefinitions    WeaponDefinition from \"%s\" was loaded\n", path);
            XmlElement const* element = rootElement->FirstChildElement();
            while (element != nullptr)
            {
                WeaponDefinition mapDef = WeaponDefinition(*element);
                WeaponDefinition::s_definitions.push_back(mapDef);
                element = element->NextSiblingElement();
            }
        }
        else
        {
            printf("WeaponDefinition::LoadDefinitions    WeaponDefinition from \"%s\"was invalid (missing root element)\n", path);
        }
    }
    else
    {
        printf("WeaponDefinition::LoadDefinitions    Failed to load WeaponDefinition from \"%s\"\n", path);
    }
}

void WeaponDefinition::ClearDefinitions()
{
    s_definitions.clear();
}

const WeaponDefinition* WeaponDefinition::GetByName(const std::string& name)
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

WeaponDefinition::WeaponDefinition(XmlElement const& weaponDefElement)
{
    m_name                       = ParseXmlAttribute(weaponDefElement, "name", m_name);
    m_refireTime                 = ParseXmlAttribute(weaponDefElement, "refireTime", m_refireTime);
    m_rayCount                   = ParseXmlAttribute(weaponDefElement, "rayCount", m_rayCount);
    m_rayCone                    = ParseXmlAttribute(weaponDefElement, "rayCone", m_rayCone);
    m_rayRange                   = ParseXmlAttribute(weaponDefElement, "rayRange", m_rayRange);
    m_rayDamage                  = ParseXmlAttribute(weaponDefElement, "rayDamage", m_rayDamage);
    m_rayImpulse                 = ParseXmlAttribute(weaponDefElement, "rayImpulse", m_rayImpulse);
    m_projectileCount            = ParseXmlAttribute(weaponDefElement, "projectileCount", m_projectileCount);
    m_projectileCone             = ParseXmlAttribute(weaponDefElement, "projectileCone", m_projectileCone);
    m_projectileSpeed            = ParseXmlAttribute(weaponDefElement, "projectileSpeed", m_projectileSpeed);
    m_projectileActor            = ParseXmlAttribute(weaponDefElement, "projectileActor", m_projectileActor);
    m_meleeCount                 = ParseXmlAttribute(weaponDefElement, "meleeCount", m_meleeCount);
    m_meleeArc                   = ParseXmlAttribute(weaponDefElement, "meleeArc", m_meleeArc);
    m_meleeRange                 = ParseXmlAttribute(weaponDefElement, "meleeRange", m_meleeRange);
    m_meleeDamage                = ParseXmlAttribute(weaponDefElement, "meleeDamage", m_meleeDamage);
    m_meleeImpulse               = ParseXmlAttribute(weaponDefElement, "meleeImpulse", m_meleeImpulse);
    const XmlElement* hudElement = FindChildElementByName(weaponDefElement, "HUD");
    if (hudElement)
    {
        printf("                                    ‖ Loading Hud Information\n");
        m_hud = new Hud(*hudElement);
    }
    const XmlElement* soundElement = FindChildElementByName(weaponDefElement, "HUD");
    if (soundElement)
    {
        printf("                                    ‖ Loading Sound Information\n");
        if (soundElement->ChildElementCount() > 0)
        {
            XmlElement const* element = soundElement->FirstChildElement();
            while (element != nullptr)
            {
                Sound sound = Sound(*element);
                m_sounds.push_back(sound);
                element = element->NextSiblingElement();
            }
        }
    }

    printf("WeaponDefinition::WeaponDefinition    — Create Definition \"%s\" \n", m_name.c_str());
}

Sound* WeaponDefinition::GetSoundByName(std::string const& soundName)
{
    for (Sound& sound : m_sounds)
    {
        if (sound.m_name == soundName)
            return &sound;
    }
    return nullptr;
}
