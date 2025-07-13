#pragma once

#include <entt/entity/fwd.hpp>

class GameWorld;

struct ComponentPower {
    int32_t m_power;
};

struct ComponentShield {
    float m_radius;
};

class SystemShield {
    public:
        static void Update(GameWorld& world, uint32_t ticks);
};

