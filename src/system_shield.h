#pragma once

#include <moth/ecs/world.h>
#include <algorithm>
#include "gamedata_entity.h"
#include <moth/graphics/utils/vector.h>
#include <cstdint>

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
    // the shield swells for a moment on each absorb, from the scale its sprite started with
    uint32_t m_pulseLeft = 0;
    moth::gfx::FloatVec2 m_baseScale{ 0.0f, 0.0f };
};

// A bullet the shield has absorbed, pulling into the shield's centre before it goes. It is no longer a projectile and
// has no body, so it can do no harm on the way.
struct ComponentAbsorbing {
    moth::ecs::Entity m_shield = moth::ecs::kNullEntity;
    moth::gfx::FloatVec2 m_from;
    uint32_t m_elapsed = 0;
};

class SystemShield {
public:
    static void Update(GameWorld& world, uint32_t ticks);
};
