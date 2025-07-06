#pragma once

#include <entt/entity/fwd.hpp>
#include <moth_ui/utils/vector.h>

struct ComponentPosition {
    moth_ui::FloatVec2 m_position;
    moth_ui::FloatVec2 m_lastPosition;
};

struct ComponentVelocity {
    moth_ui::FloatVec2 m_velocity;
};

class SystemMovement {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
};
