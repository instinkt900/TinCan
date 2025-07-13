#pragma once

#include <entt/entity/fwd.hpp>

class GameWorld;

struct ComponentLifetime {
    uint32_t m_msAlive = 0;
    uint32_t m_msLeft = 1000;
};

class SystemLifetime {
public:
    static void Update(GameWorld& world, uint32_t ticks);
};
