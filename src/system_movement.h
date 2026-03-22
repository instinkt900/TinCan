#pragma once

#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <moth_graphics/utils/vector.h>

class GameWorld;

struct ComponentParenting {
    entt::entity m_parent = entt::null;
    moth_graphics::FloatVec2 m_offset;

    ComponentParenting(entt::entity parent, moth_graphics::FloatVec2 const& offset = {})
        : m_parent(parent)
        , m_offset(offset) {}
};

struct ComponentPosition {
    moth_graphics::FloatVec2 m_position;
    moth_graphics::FloatVec2 m_lastPosition;

    explicit ComponentPosition(moth_graphics::FloatVec2 const& initialPosition)
        : m_position(initialPosition)
        , m_lastPosition(initialPosition) {}
};

struct ComponentVelocity {
    moth_graphics::FloatVec2 m_velocity;
};

class SystemMovement {
public:
    static void Update(GameWorld& world, uint32_t ticks);
};
