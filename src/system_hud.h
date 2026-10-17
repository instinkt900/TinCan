#pragma once

#include "system_drawable.h"
#include <moth/ecs/world.h>
#include <moth/graphics/graphics/igraphics.h>
#include <cstdint>
#include <memory>

class GameWorld;

// What the on-screen display keeps between frames: the sprite it draws the lives with, and its clock for the glow.
struct ComponentHud {
    std::unique_ptr<ComponentSprite> m_lifeIcon;
    uint32_t m_time = 0;
    uint32_t m_lifeFlashLeft = 0; // the newest life icon flashes while this runs
    uint32_t m_powerTickLeft = 0; // the segment taking power lights up while this runs
};

// The on-screen display: the lives top left, the score top right and the power meter bottom left, drawn in the
// play field over the world.
class SystemHud {
public:
    // makes the newest life icon flash, for an extra life
    static void FlashNewLife(moth::ecs::World& registry);
    // lights up the meter's filling segment, for power coming in
    static void TickPower(moth::ecs::World& registry);
    static void Update(GameWorld& world, uint32_t ticks);
    static void Draw(GameWorld& world, moth::gfx::IGraphics& graphics, moth::gfx::IFont& font);
};
