#pragma once

#include <moth/ecs/world.h>
#include <algorithm>
#include "gamedata_entity.h"

class GameWorld;

struct ComponentPower {
    float m_power = 0.0f;
    float m_maxPower = 0.0f; // power over this is lost

    // adds power from any source, keeping it under the most it can hold
    void Add(float amount) { m_power = std::clamp(m_power + amount, 0.0f, m_maxPower); }
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
