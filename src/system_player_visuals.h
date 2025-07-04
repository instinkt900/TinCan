#pragma once

#include <entt/entity/fwd.hpp>

class SystemPlayerVisuals {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
};
