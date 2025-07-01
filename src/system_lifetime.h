#pragma once

#include <entt/entity/fwd.hpp>

struct ComponentLifetime {
    uint32_t m_lifetime;
};

class SystemLifetime {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
};
