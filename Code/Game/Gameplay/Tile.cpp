#include "Tile.hpp"

#include "Game/Definition/TileDefinition.hpp"

Tile::~Tile()
{
}

Tile::Tile(IntVec2 tileCoords, TileDefinition* tileDefinition): m_tileCoords(tileCoords), m_definition(tileDefinition)
{
}

void Tile::SetTileDefinition(TileDefinition* tileDefinition)
{
    m_definition = tileDefinition;
}

TileDefinition* Tile::GetTileDefinition() const
{
    return m_definition;
}

int Tile::GetTileHealth() const
{
    return m_health;
}

bool Tile::IsTileSolid() const
{
    return m_definition->m_isSolid;
}

AABB3& Tile::GetBounds()
{
    return m_bounds;
}

Tile& Tile::SetBounds(AABB3 newBounds)
{
    m_bounds = newBounds;
    return *this;
}

Tile& Tile::SetTileCoords(IntVec2 newCoords)
{
    m_tileCoords = newCoords;
    return *this;
}

IntVec2 Tile::GetTileCoords() const
{
    return m_tileCoords;
}
