#include "system_lifetime.h"
#include "component_entity.h"
#include "system_group.h"
#include "system_movement.h"
#include "tags.h"
#include <moth/ecs/world.h>
#include <spdlog/spdlog.h>
#include "game_world.h"

void UpdateLifetimes(moth::ecs::World& registry, uint32_t ticks, std::set<moth::ecs::Entity>& deadEntities) {
    auto view = registry.View<ComponentLifetime>();
    for (auto [entity, lifetime] : view.each()) {
        if (lifetime.m_msLeft <= ticks) {
            deadEntities.insert(entity);
            registry.GetOrEmplace<EscapedTag>(entity);
        } else {
            lifetime.m_msLeft -= ticks;
            lifetime.m_msAlive += ticks;
        }
    }
}

void UpdateDead(moth::ecs::World& registry, std::set<moth::ecs::Entity>& deadEntities) {
    auto view = registry.View<DeadTag>();
    for (auto [entity] : view.each()) {
        deadEntities.insert(entity);

        // if a parent dies, kill all children too
        if (auto const* details = registry.TryGet<ComponentEntity>(entity)) {
            for (auto child : details->m_children) {
                deadEntities.insert(child);
            }
        }
        // update any parent if a child dies
        if (auto const* parenting = registry.TryGet<ComponentParenting>(entity)) {
            if (registry.Valid(parenting->m_parent)) {
                auto& parentDetails = registry.Get<ComponentEntity>(parenting->m_parent);
                parentDetails.m_children.erase(entity);
                if (parentDetails.m_children.empty()) {
                    // if no children are left, destroy the parent
                    deadEntities.insert(parenting->m_parent);
                }
            }
        }
    }
}

void SystemLifetime::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    std::set<moth::ecs::Entity> deadEntities;

    UpdateLifetimes(registry, ticks, deadEntities);
    UpdateDead(registry, deadEntities);

    for (auto entity : deadEntities) {
        // notify any groups of a dead member. every route out of the world passes through here, so this is
        // the one place a member is removed from its group, and EscapedTag says which of the two it was
        if (auto const* groupId = registry.TryGet<ComponentGroupId>(entity)) {
            auto const condition = registry.Has<EscapedTag>(entity) ? GroupEndCondition::Escaped
                                                                   : GroupEndCondition::Killed;
            SystemGroup::RemoveMember(registry, groupId->m_group, entity, condition);
        }
        registry.Destroy(entity);
    }
}
