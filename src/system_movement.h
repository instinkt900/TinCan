#pragma once

#include <moth/ecs/world.h>
#include <moth/graphics/utils/vector.h>
#include "gamedata_movement.h"

class GameWorld;

struct ComponentParenting {
    moth::ecs::Entity m_parent = moth::ecs::kNullEntity;
    moth::gfx::FloatVec2 m_offset;

    ComponentParenting(moth::ecs::Entity parent, moth::gfx::FloatVec2 const& offset = {})
        : m_parent(parent)
        , m_offset(offset) {}
};

struct ComponentPosition {
    moth::gfx::FloatVec2 m_position;
    moth::gfx::FloatVec2 m_lastPosition;

    explicit ComponentPosition(moth::gfx::FloatVec2 const& initialPosition)
        : m_position(initialPosition)
        , m_lastPosition(initialPosition) {}
};

struct ComponentVelocity {
    moth::gfx::FloatVec2 m_velocity;
};

struct ComponentMovement {
    float m_maxSpeed = 400.0f;
    float m_acceleration = 0.0f;
    float m_deceleration = 0.0f;

    ComponentMovement() = default;
    explicit ComponentMovement(MovementData const& data)
        : m_maxSpeed(data.max_speed)
        , m_acceleration(data.acceleration)
        , m_deceleration(data.deceleration) {}
};

class SystemMovement {
public:
    static void Update(GameWorld& world, uint32_t ticks);
};
