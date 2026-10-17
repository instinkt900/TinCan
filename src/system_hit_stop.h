#pragma once

#include <moth/ecs/world.h>
#include <cstdint>

class GameWorld;

// How much longer the game stays frozen. One entity holds it, so a big moment can ask for a hit-stop from
// anywhere the registry reaches.
struct ComponentHitStop {
    uint32_t m_msLeft = 0;
};

class SystemHitStop {
public:
    // freezes the game for this long, unless a longer hit-stop is already running
    static void Start(moth::ecs::World& registry, uint32_t milliseconds);
    // counts the hit-stop down, and says whether the rest of this frame's update should be skipped
    static bool Freeze(GameWorld& world, uint32_t ticks);
};
