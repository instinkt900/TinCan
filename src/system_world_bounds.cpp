#include "system_world_bounds.h"
#include "system_movement.h"
#include "tags.h"
#include <entt/entt.hpp>
#include "game_world.h"

void KillOutOfBounds(entt::registry& registry, entt::entity entity, ComponentBounds const& bounds,
                     ComponentPosition& position) {
    registry.get_or_emplace<DeadTag>(entity);
}

void LimitOutOfBounds(entt::registry& registry, entt::entity entity, ComponentBounds const& bounds,
                      ComponentPosition& position) {
    auto delta = position.m_position - position.m_lastPosition;
    if (position.m_position.x < bounds.m_bounds.left() || position.m_position.x > bounds.m_bounds.right()) {
        delta.x = 0;
    }
    if (position.m_position.y < bounds.m_bounds.top() || position.m_position.y > bounds.m_bounds.bottom()) {
        delta.y = 0;
    }
    position.m_position = position.m_lastPosition + delta;
}

void HandleOutOfBounds(entt::registry& registry, entt::entity entity, ComponentBounds const& bounds,
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
    auto view = registry.view<ComponentBounds, ComponentPosition>();

    for (auto [entity, bounds, position] : view.each()) {
        if (!moth_ui::IsInRect(position.m_position, bounds.m_bounds)) {
            HandleOutOfBounds(registry, entity, bounds, position);
        }
    }
}
