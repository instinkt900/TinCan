#include "system_group.h"
#include "system_movement.h"
#include "system_pickup.h"
#include "tags.h"
#include <moth/ecs/world.h>
#include <spdlog/spdlog.h>
#include "game_world.h"
#include "gamedata_pickup.h"

void SystemGroup::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto const& gamedata = world.GetGameData();
    auto view = registry.View<ComponentGroup>();

    for (auto [entity, group] : view.each()) {
        if (group.m_memberCount <= 0) {
            registry.Emplace<DeadTag>(entity);

            if (group.m_drop.valid()) {
                SystemPickup::CreatePickup(registry, group.m_position, *group.m_drop, gamedata);
            }
        }
    }
}

moth::ecs::Entity SystemGroup::CreateGroup(moth::ecs::World& registry, DataRef<PickupData> drop) {
    auto entity = registry.Create();
    auto& group = registry.Emplace<ComponentGroup>(entity);
    group.m_memberCount = 0;
    group.m_drop = drop;
    return entity;
}

void SystemGroup::AddMember(moth::ecs::World& registry, moth::ecs::Entity group, moth::ecs::Entity member) {
    if (auto* groupComp = registry.TryGet<ComponentGroup>(group)) {
        auto& groupRef = registry.Emplace<ComponentGroupId>(member);
        groupRef.m_group = group;
        groupComp->m_memberCount++;
    }
}

void SystemGroup::RemoveMember(moth::ecs::World& registry, moth::ecs::Entity group, moth::ecs::Entity member, GroupEndCondition condition) {
    if (auto* groupComp = registry.TryGet<ComponentGroup>(group)) {
        groupComp->m_memberCount--;

        auto const* memberPos = registry.TryGet<ComponentPosition>(member);
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
