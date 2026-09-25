#include "system_world_bounds.h"
#include "system_movement.h"
#include "tags.h"
#include <moth/ecs/world.h>
#include "game_world.h"

namespace {
    // how far past the screen edge an active enemy may go before it counts as escaped
    float constexpr ScreenExitBorder = 64.0f;
}

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

    auto const worldSize = static_cast<moth::gfx::FloatVec2>(world.GetWorldSize());
    moth::gfx::FloatRect const screenRect{ { 0.0f, 0.0f }, worldSize };
    auto exitRect = screenRect;
    exitRect.topLeft -= { ScreenExitBorder, ScreenExitBorder };
    exitRect.bottomRight += { ScreenExitBorder, ScreenExitBorder };
    auto exitView = registry.View<ComponentScreenExit, ComponentPosition>(moth::ecs::Exclude<DeadTag>);
    for (auto [entity, screenExit, position] : exitView.each()) {
        if (!screenExit.m_active) {
            screenExit.m_active = moth::core::IsInRect(position.m_position, screenRect);
        } else if (!moth::core::IsInRect(position.m_position, exitRect)) {
            registry.Emplace<DeadTag>(entity);
            registry.GetOrEmplace<EscapedTag>(entity);
        }
    }
}
