#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/FloatRange.hpp"


class WeaponDefinition
{
public:
    static std::vector<WeaponDefinition> s_definitions;
    static void                          LoadDefinitions(const char* path);
    static void                          ClearDefinitions();
    static const WeaponDefinition*       GetByName(const std::string& name);

public:
    WeaponDefinition(XmlElement const& mapDefElement);

public:
    std::string m_name            = "Default";
    float       m_refireTime      = 0.f;
    int         m_rayCount        = 0;
    float       m_rayCone         = 0.f;
    float       m_rayRange        = 0.f;
    FloatRange  m_rayDamage       = FloatRange(0.0f, 0.0f);
    float       m_rayImpulse      = 0.f;
    int         m_projectileCount = 0;
    float       m_projectileCone  = 0.f;
    float       m_projectileSpeed = 0.f;
    std::string m_projectileActor = "";
    int         m_meleeCount      = 0;
    float       m_meleeArc        = 0.f;
    float       m_meleeRange      = 0.f;
    FloatRange  m_meleeDamage     = FloatRange(0.0f, 0.0f);
    float       m_meleeImpulse    = 0.f;
};
