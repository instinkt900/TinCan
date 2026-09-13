#pragma once

#include "tiled_map.h"
#include <moth/ecs/world.h>
#include <moth/graphics/graphics/igraphics.h>

class GameWorld;

struct ComponentMapScroll {
    TiledMap m_map;
    float m_cameraTop = 0; // map pixel row currently at the top of the screen
};

class SystemMap {
public:
    // creates the map entity along with the camera
    static void InitMap(moth::ecs::World& registry, TiledMap map, moth::gfx::IntVec2 const& viewSize);
    static void Update(GameWorld& world, uint32_t ticks);
    static void Draw(GameWorld& world, moth::gfx::IGraphics& graphics);
};
