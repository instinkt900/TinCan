#pragma once

#include <entt/entity/fwd.hpp>

struct ComponentPower {
    int32_t m_power;
};

struct ComponentShield {
    float m_radius;
};

class SystemShield {
    public:
        static void Update(entt::registry& registry, uint32_t ticks);
};

