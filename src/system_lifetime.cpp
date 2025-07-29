#include "system_lifetime.h"
#include "component_entity.h"
#include "system_group.h"
#include "system_movement.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>
#include "game_world.h"

void UpdateLifetimes(entt::registry& registry, uint32_t ticks, std::set<entt::entity>& deadEntities) {
    auto view = registry.view<ComponentLifetime>();
    for (auto [entity, lifetime] : view.each()) {
        if (lifetime.m_msLeft <= ticks) {
            deadEntities.insert(entity);
            if (auto const* groupId = registry.try_get<ComponentGroupId>(entity)) {
                SystemGroup::RemoveMember(registry, groupId->m_group, entity, GroupEndCondition::Escaped);
            }
        } else {
            lifetime.m_msLeft -= ticks;
            lifetime.m_msAlive += ticks;
        }
    }
}

void UpdateDead(entt::registry& registry, std::set<entt::entity>& deadEntities) {
    auto view = registry.view<DeadTag>();
    for (auto [entity] : view.each()) {
        deadEntities.insert(entity);

        // if a parent dies, kill all children too
        if (auto* details = registry.try_get<ComponentEntity>(entity)) {
            for (auto child : details->m_children) {
                deadEntities.insert(child);
            }
        }
    }
}

void SystemLifetime::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    std::set<entt::entity> deadEntities;

    UpdateLifetimes(registry, ticks, deadEntities);
    UpdateDead(registry, deadEntities);

    for (auto entity : deadEntities) {
        // update any parent if a child dies
        if (auto const* parenting = registry.try_get<ComponentParenting>(entity)) {
            if (registry.valid(parenting->m_parent)) {
                auto& parentDetails = registry.get<ComponentEntity>(parenting->m_parent);
                parentDetails.m_children.erase(entity);
            }
        }
        // notify any groups of a dead member
        if (auto const* groupId = registry.try_get<ComponentGroupId>(entity)) {
            SystemGroup::RemoveMember(registry, groupId->m_group, entity, GroupEndCondition::Killed);
        }
        registry.destroy(entity);
    }
}
