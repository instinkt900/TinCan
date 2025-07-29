#pragma once

#include <entt/entity/fwd.hpp>

class GameWorld;

struct ComponentPower {
    float m_power;
};

struct ComponentShield {
};

class SystemShield {
public:
    static void Update(GameWorld& world, uint32_t ticks);
};
