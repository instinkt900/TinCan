#include "system_lifetime.h"
#include "component_entity.h"
#include "system_enemy_spawner.h"
#include "system_group.h"
#include "system_movement.h"
#include "tags.h"
#include <moth/ecs/world.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <vector>
#include "game_world.h"

void UpdateLifetimes(moth::ecs::World& registry, uint32_t ticks) {
    auto view = registry.View<ComponentLifetime>(moth::ecs::Exclude<DeadTag>);
    for (auto [entity, lifetime] : view.each()) {
        if (lifetime.m_msLeft <= ticks) {
            // marked dead rather than removed here, so the dead pass takes its children with it
            registry.GetOrEmplace<DeadTag>(entity);
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

// Spawns the death spawns of a killed enemy around where it died. Runs here rather than where the damage is
// dealt, because that happens inside views over the pools the new enemies are added to.
void SpawnDeathSpawns(GameWorld& world, moth::ecs::Entity entity, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto const& gamedata = world.GetGameData();
    auto const* deathSpawns = registry.TryGet<ComponentDeathSpawns>(entity);
    auto const* position = registry.TryGet<ComponentPosition>(entity);
    if (deathSpawns == nullptr || deathSpawns->m_spawns == nullptr || position == nullptr) {
        return;
    }

    // copied out, because spawning adds to the pools these components live in
    auto const* spawns = deathSpawns->m_spawns;
    auto const origin = position->m_position;
    // pieces with no behaviour carry on at the speed the parent was going, taken from its last step so that
    // it does not matter whether a behaviour or a velocity was moving it
    moth::gfx::FloatVec2 velocity{ 0.0f, 0.0f };
    if (ticks > 0) {
        velocity = (position->m_position - position->m_lastPosition) * (1000.0f / static_cast<float>(ticks));
    }
    // the pieces join the parent's groups, so a group waits for them before it drops
    std::vector<moth::ecs::Entity> parentGroups;
    if (auto const* groupId = registry.TryGet<ComponentGroupId>(entity)) {
        parentGroups = groupId->m_groups;
    }

    for (auto const& spawn : *spawns) {
        EnemySpawnDescription description;
        description.enemy = gamedata.Get<EnemyData>(spawn.enemy_type);
        if (description.enemy == nullptr) {
            spdlog::error("Death spawn references unknown enemy {}", spawn.enemy_type);
            continue;
        }
        if (spawn.behaviour.valid()) {
            description.behaviour = spawn.behaviour.get();
        }
        description.behaviourOverride.speed = spawn.speed;
        description.behaviourOverride.rotation = spawn.behaviour_rotation;
        description.rotationStep = spawn.behaviour_rotation_step;
        description.count = std::max(1, spawn.count);
        description.formation = spawn.formation;
        description.spacing = spawn.spacing;
        description.scale = { spawn.scale_x, spawn.scale_y };
        if (spawn.drop.valid()) {
            description.drop = spawn.drop;
        }
        SystemEnemySpawner::SpawnFormation(registry, gamedata, description, origin, 0.0f, velocity,
                                           parentGroups);
    }
}

void SystemLifetime::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    std::set<moth::ecs::Entity> deadEntities;

    UpdateLifetimes(registry, ticks);
    UpdateDead(registry, deadEntities);

    for (auto entity : deadEntities) {
        bool const escaped = registry.Has<EscapedTag>(entity);
        // any kill spawns the enemy's death spawns. they join its groups before it leaves them, so a group
        // is never left empty while its pieces are still coming
        if (!escaped) {
            SpawnDeathSpawns(world, entity, ticks);
        }

        // notify any groups of a dead member. every route out of the world passes through here, so this is
        // the one place a member is removed from its group, and EscapedTag says which of the two it was
        if (auto const* groupId = registry.TryGet<ComponentGroupId>(entity)) {
            auto const condition = escaped ? GroupEndCondition::Escaped : GroupEndCondition::Killed;
            for (auto const group : groupId->m_groups) {
                SystemGroup::RemoveMember(registry, group, entity, condition);
            }
        }
        registry.Destroy(entity);
    }
}
