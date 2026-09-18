#pragma once

#include <moth/ecs/world.h>
#include "gamedata_entity.h"

class GameWorld;

struct ComponentPower {
    float m_power;
};

struct ComponentShield {
    // The affinity the shield visuals were last switched to; a mismatch with the
    // owner's affinity drives the transition animation.
    Affinity m_shownAffinity = Affinity::None;
};

class SystemShield {
public:
    static void Update(GameWorld& world, uint32_t ticks);
};
