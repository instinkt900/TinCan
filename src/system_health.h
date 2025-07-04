#pragma once

#include <entt/entity/fwd.hpp>
#include <functional>

class SystemHealth {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
};
