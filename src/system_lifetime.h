#pragma once

#include <entt/entity/fwd.hpp>

struct ComponentLifetime {
    uint32_t m_msAlive = 0;
    uint32_t m_msLeft = 1000;
};

class SystemLifetime {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
};
