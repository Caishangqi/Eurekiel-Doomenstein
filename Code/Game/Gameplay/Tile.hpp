#pragma once
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVec2.hpp"

class TileDefinition;

    struct Tile
    {
        ~Tile();
        Tile(IntVec2 tileCoords, TileDefinition* tileDefinition);
        Tile() = default;

    public:
        void            SetTileDefinition(TileDefinition* tileDefinition);
        TileDefinition* GetTileDefinition() const;
        int             GetTileHealth() const;
        bool            IsTileSolid() const;
        AABB3&          GetBounds();
        Tile&           SetBounds(AABB3 newBounds);
        Tile&           SetTileCoords(IntVec2 newCoords);
        IntVec2         GetTileCoords() const;

    private:
        AABB3           m_bounds;
        TileDefinition* m_definition = nullptr;
        IntVec2         m_tileCoords = IntVec2::INVALID;
        int             m_health     = 1;
    };

