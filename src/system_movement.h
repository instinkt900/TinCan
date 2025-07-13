#pragma once

#include <entt/entity/fwd.hpp>
#include <canyon/utils/vector.h>

struct ComponentPosition {
    canyon::FloatVec2 m_position;
    canyon::FloatVec2 m_lastPosition;
};

struct ComponentVelocity {
    canyon::FloatVec2 m_velocity;
};

class SystemMovement {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
};
