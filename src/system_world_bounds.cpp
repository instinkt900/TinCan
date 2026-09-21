#include "system_world_bounds.h"
#include "system_movement.h"
#include "tags.h"
#include <moth/ecs/world.h>
#include "game_world.h"

void KillOutOfBounds(moth::ecs::World& registry, moth::ecs::Entity entity, ComponentBounds const& bounds,
                     ComponentPosition& position) {
    registry.GetOrEmplace<DeadTag>(entity);
    // leaving the play area is never the player's doing, so it counts as an escape and its group earns
    // no drop for it
    registry.GetOrEmplace<EscapedTag>(entity);
}

void LimitOutOfBounds(moth::ecs::World& registry, moth::ecs::Entity entity, ComponentBounds const& bounds,
                      ComponentPosition& position) {
    auto delta = position.m_position - position.m_lastPosition;
    bool const blockedX =
        position.m_position.x < bounds.m_bounds.left() || position.m_position.x > bounds.m_bounds.right();
    bool const blockedY =
        position.m_position.y < bounds.m_bounds.top() || position.m_position.y > bounds.m_bounds.bottom();
    if (blockedX) {
        delta.x = 0;
    }
    if (blockedY) {
        delta.y = 0;
    }
    position.m_position = position.m_lastPosition + delta;

    // Held against an edge the entity keeps no speed in the direction it cannot
    // travel, so leaving the edge starts from a stop rather than from whatever
    // it had built up pressing into it.
    if (auto* velocity = registry.TryGet<ComponentVelocity>(entity)) {
        if (blockedX) {
            velocity->m_velocity.x = 0;
        }
        if (blockedY) {
            velocity->m_velocity.y = 0;
        }
    }
}

void HandleOutOfBounds(moth::ecs::World& registry, moth::ecs::Entity entity, ComponentBounds const& bounds,
                       ComponentPosition& position) {
    switch (bounds.m_behaviour) {
    case BoundsBehaviour::Kill:
        KillOutOfBounds(registry, entity, bounds, position);
        break;
    case BoundsBehaviour::Limit:
        LimitOutOfBounds(registry, entity, bounds, position);
        break;
    default:
        break;
    }
}
void SystemWorldBounds::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto view = registry.View<ComponentBounds, ComponentPosition>();

    for (auto [entity, bounds, position] : view.each()) {
        if (!moth::core::IsInRect(position.m_position, bounds.m_bounds)) {
            HandleOutOfBounds(registry, entity, bounds, position);
        }
    }
}
