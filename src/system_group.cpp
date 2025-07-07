#include "system_group.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

void SystemGroup::Update(entt::registry& registry, uint32_t ticks) {
    auto view = registry.view<ComponentGroup>();

    for (auto [entity, group] : view.each()) {
        if (group.m_memberCount <= 0) {
            // TODO: do something?
            spdlog::info("Group gone: {}", static_cast<int>(group.m_condition));

            registry.emplace<DeadTag>(entity);
        }
    }
}

entt::entity SystemGroup::CreateGroup(entt::registry& registry) {
    spdlog::info("New group");
    auto entity = registry.create();
    auto& group = registry.emplace<ComponentGroup>(entity);
    group.m_memberCount = 0;
    return entity;
}

void SystemGroup::AddMember(entt::registry& registry, entt::entity group, entt::entity member) {
    if (auto* groupComp = registry.try_get<ComponentGroup>(group)) {
        auto& groupRef = registry.emplace<ComponentGroupId>(member);
        groupRef.m_group = group;
        groupComp->m_memberCount++;
    }
}

void SystemGroup::RemoveMember(entt::registry& registry, entt::entity group, GroupEndCondition condition) {
    if (auto* groupComp = registry.try_get<ComponentGroup>(group)) {
        groupComp->m_memberCount--;

        if (groupComp->m_condition != condition && groupComp->m_condition != GroupEndCondition::None) {
            groupComp->m_condition = GroupEndCondition::Mixed;
        } else {
            groupComp->m_condition = condition;
        }
    }
}
