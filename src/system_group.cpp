#include "system_group.h"
#include "system_movement.h"
#include "system_pickup.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>
#include "game_world.h"

void SystemGroup::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto const& gamedata = world.GetGameData();
    auto view = registry.view<ComponentGroup>();

    for (auto [entity, group] : view.each()) {
        if (group.m_memberCount <= 0) {
            registry.emplace<DeadTag>(entity);

            if (group.m_drop.has_value()) {
                SystemPickup::CreatePickup(registry, group.m_position, group.m_drop.value(), gamedata);
            }
        }
    }
}

entt::entity SystemGroup::CreateGroup(entt::registry& registry, std::optional<std::string> drop) {
    auto entity = registry.create();
    auto& group = registry.emplace<ComponentGroup>(entity);
    group.m_memberCount = 0;
    group.m_drop = drop;
    return entity;
}

void SystemGroup::AddMember(entt::registry& registry, entt::entity group, entt::entity member) {
    if (auto* groupComp = registry.try_get<ComponentGroup>(group)) {
        auto& groupRef = registry.emplace<ComponentGroupId>(member);
        groupRef.m_group = group;
        groupComp->m_memberCount++;
    }
}

void SystemGroup::RemoveMember(entt::registry& registry, entt::entity group, entt::entity member, GroupEndCondition condition) {
    if (auto* groupComp = registry.try_get<ComponentGroup>(group)) {
        groupComp->m_memberCount--;

        auto const* memberPos = registry.try_get<ComponentPosition>(member);
        if (memberPos != nullptr) {
            groupComp->m_position = memberPos->m_position;
        }

        if (groupComp->m_condition != condition && groupComp->m_condition != GroupEndCondition::None) {
            groupComp->m_condition = GroupEndCondition::Mixed;
        } else {
            groupComp->m_condition = condition;
        }
    }
}
