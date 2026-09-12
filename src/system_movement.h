#pragma once

#include <moth/ecs/world.h>
#include <moth/graphics/utils/vector.h>

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

class SystemMovement {
public:
    static void Update(GameWorld& world, uint32_t ticks);
};
