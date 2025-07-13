#pragma once

#include <entt/entity/fwd.hpp>
#include <canyon/utils/rect.h>

class SystemWorldBounds {
    public:
        static void Update(entt::registry& registry, uint32_t ticks, canyon::FloatRect const& bounds);
};

