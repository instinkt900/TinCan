#include "system_lifetime.h"
#include "system_group.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

void UpdateLifetimes(entt::registry& registry, uint32_t ticks, std::set<entt::entity>& deadEntities) {
    auto view = registry.view<ComponentLifetime>();
    for (auto [entity, lifetime] : view.each()) {
        if (lifetime.m_msLeft <= ticks) {
            deadEntities.insert(entity);
            if (auto const* groupId = registry.try_get<ComponentGroupId>(entity)) {
                SystemGroup::RemoveMember(registry, groupId->m_group, GroupEndCondition::Escaped);
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
        if (auto const* groupId = registry.try_get<ComponentGroupId>(entity)) {
            SystemGroup::RemoveMember(registry, groupId->m_group, GroupEndCondition::Killed);
        }
    }
}

void SystemLifetime::Update(entt::registry& registry, uint32_t ticks) {
    std::set<entt::entity> deadEntities;

    UpdateLifetimes(registry, ticks, deadEntities);
    UpdateDead(registry, deadEntities);

    for (auto entity : deadEntities) {
        registry.destroy(entity);
    }
}
