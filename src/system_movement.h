#pragma once

#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <canyon/utils/vector.h>

class GameWorld;

struct ComponentParenting {
    entt::entity m_parent = entt::null;
    canyon::FloatVec2 m_offset;

    ComponentParenting(entt::entity parent, canyon::FloatVec2 const& offset = {})
        : m_parent(parent)
        , m_offset(offset) {}
};

struct ComponentPosition {
    canyon::FloatVec2 m_position;
    canyon::FloatVec2 m_lastPosition;

    explicit ComponentPosition(canyon::FloatVec2 const& initialPosition)
        : m_position(initialPosition)
        , m_lastPosition(initialPosition) {}
};

struct ComponentVelocity {
    canyon::FloatVec2 m_velocity;
};

class SystemMovement {
public:
    static void Update(GameWorld& world, uint32_t ticks);
};
