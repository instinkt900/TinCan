#include "system_score.h"
#include "system_enemy_spawner.h"
#include "component_body.h"
#include "component_contact.h"
#include "component_death_sprite.h"
#include "component_entity.h"
#include "component_health.h"
#include "formation_offsets.h"
#include "system_behaviour.h"
#include "system_movement.h"
#include "system_drawable.h"
#include "system_group.h"
#include "system_lifetime.h"
#include "system_weapon.h"
#include "system_world_bounds.h"
#include "tags.h"
#include <moth/ecs/world.h>
#include <moth/profile/profiler.h>

moth::ecs::Entity SystemEnemySpawner::SpawnEnemy(moth::ecs::World& registry, EnemyData const& data,
                                            BehaviourData const& behaviour, moth::gfx::FloatVec2 const& position,
                                            GameData const& gamedata) {
    MOTH_PROFILE_SCOPE("SpawnEnemy");
    auto rootEntity = registry.Create();
    auto& rootDetails = registry.Emplace<ComponentEntity>(rootEntity, Team::Enemy, data.affinity);
    registry.Emplace<ComponentPosition>(rootEntity, position);
    registry.Emplace<ComponentVelocity>(rootEntity);

    // every enemy is culled the same way: by its lifetime, or by leaving the screen once it has been on it
    registry.Emplace<ComponentLifetime>(rootEntity, 0u, data.lifetime);
    registry.Emplace<ComponentScreenExit>(rootEntity);

    if (data.radius > 0.0f) {
        registry.Emplace<ComponentBody>(rootEntity, data.radius);
    }

    if (data.health > 0.0f) {
        registry.Emplace<ComponentHealth>(rootEntity, data.health);
        registry.Emplace<TargetTag>(rootEntity);
    }

    if (data.sprite.valid()) {
        registry.Emplace<ComponentSprite>(rootEntity, *data.sprite, data.spin_scale);
    }

    if (data.weapon.valid()) {
        SystemWeapon::InitWeapon(registry, rootEntity, data.weapon.ref_name(), *data.weapon, gamedata,
                                 true);
    }

    if (data.death_sprite.valid()) {
        registry.Emplace<ComponentDeathSprite>(rootEntity, data.death_sprite.get());
    }

    if (data.contact.has_value()) {
        registry.Emplace<ComponentContact>(rootEntity, *data.contact);
    }

    if (data.large) {
        registry.Emplace<LargeTag>(rootEntity);
    }

    if (data.score > 0) {
        registry.Emplace<ComponentScoreValue>(rootEntity, data.score);
    }

    if (!data.death_spawns.empty()) {
        registry.Emplace<ComponentDeathSpawns>(rootEntity, &data.death_spawns);
    }

    {
        MOTH_PROFILE_SCOPE("Enemy sections");
        for (auto const& [sectionName, section] : data.sections) {
            auto childEntity = registry.Create();
            registry.Emplace<TargetTag>(childEntity);
            registry.Emplace<ComponentParenting>(childEntity, rootEntity,
                                                 static_cast<moth::gfx::FloatVec2>(section.offset));
            registry.Emplace<ComponentPosition>(childEntity,
                                                position + static_cast<moth::gfx::FloatVec2>(section.offset));
            if (section.radius > 0.0f) {
                registry.Emplace<ComponentBody>(childEntity, section.radius);
            }
            if (section.health > 0) {
                registry.Emplace<ComponentHealth>(childEntity, section.health);
            }
            if (section.sprite.valid()) {
                // a section spins only if its own sprite asks for it: sections are parented by offset
                // and draw at angle 0, so the root's spin scale would set them turning on their own
                registry.Emplace<ComponentSprite>(childEntity, *section.sprite);
            }

            if (section.weapon.valid()) {
                SystemWeapon::InitWeapon(registry, childEntity, section.weapon.ref_name(), *section.weapon,
                                         gamedata, true);
            }

            if (section.death_sprite.valid()) {
                registry.Emplace<ComponentDeathSprite>(childEntity, section.death_sprite.get());
            }

            if (section.contact.has_value()) {
                registry.Emplace<ComponentContact>(childEntity, *section.contact);
            }

            rootDetails.m_children.insert(childEntity);
        }
    }

    registry.Emplace<ComponentBehaviour>(rootEntity, behaviour);

    return rootEntity;
}

void SystemEnemySpawner::SpawnFormation(moth::ecs::World& registry, GameData const& gamedata,
                                        EnemySpawnDescription const& spawn,
                                        moth::gfx::FloatVec2 const& origin, float rotation,
                                        moth::gfx::FloatVec2 const& idleVelocity,
                                        std::vector<moth::ecs::Entity> const& extraGroups) {
    MOTH_PROFILE_SCOPE("Spawn formation");
    if (spawn.enemy == nullptr) {
        return;
    }

    // the members share a group, so the drop appears once the whole formation has been killed
    auto group = moth::ecs::kNullEntity;
    if (spawn.count > 1 || spawn.drop.valid()) {
        group = SystemGroup::CreateGroup(registry, spawn.drop);
    }

    uint32_t phaseIndex = 0;
    auto const offsets =
        GetFormationOffsets(gamedata, spawn.formation, spawn.count, spawn.spacing, spawn.scale, rotation);
    for (auto const& offset : offsets) {
        auto const position = origin + offset;
        // each member gets its own copy of the behaviour, because the rotation steps along the formation so
        // that a group fans out
        auto memberBehaviour = spawn.behaviour != nullptr ? *spawn.behaviour : BehaviourData{};
        auto memberOverride = spawn.behaviourOverride;
        memberOverride.rotation += spawn.rotationStep * static_cast<float>(phaseIndex);
        ApplyBehaviourOverride(memberBehaviour, memberOverride);

        auto enemy = SpawnEnemy(registry, *spawn.enemy, memberBehaviour, position, gamedata);

        // a Phase modifier staggers the formation by each member's place in it
        registry.Get<ComponentBehaviour>(enemy).m_phaseIndex = phaseIndex++;

        if (spawn.behaviour == nullptr) {
            registry.Get<ComponentVelocity>(enemy).m_velocity = idleVelocity;
        }

        if (group != moth::ecs::kNullEntity) {
            SystemGroup::AddMember(registry, group, enemy);
        }
        for (auto const extraGroup : extraGroups) {
            SystemGroup::AddMember(registry, extraGroup, enemy);
        }
    }
}
