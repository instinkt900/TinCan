#include "system_enemy_spawner.h"
#include "game_layer.h"
#include "system_behaviour.h"
#include "system_group.h"
#include "system_health.h"
#include "system_projectile.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_drawable.h"
#include "system_weapon.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

void SystemEnemySpawner::Update(entt::registry& registry, uint32_t ticks, Gamedata const& gamedata) {
    auto const* enemyDatabase = gamedata.GetEnemyDatabase();
    if (enemyDatabase == nullptr) {
        return;
    }

    auto const* weaponDatabase = gamedata.GetWeaponDatabase();
    if (weaponDatabase == nullptr) {
        return;
    }

    auto view = registry.view<ComponentEnemySpawner, ComponentPosition>();

    for (auto [entity, spawner, spawnerPosition] : view.each()) {
        if (spawner.m_count > 0) {
            if (spawner.m_cooldown > 0) {
                spawner.m_cooldown -= static_cast<int32_t>(ticks);
            }

            if (spawner.m_cooldown <= 0) {
                spawner.m_groupCount = spawner.m_maxGroupCount;
                if (spawner.m_maxGroupCount > 1) {
                    spawner.m_currentGroupEntity = entt::null;
                }
                spawner.m_cooldown += spawner.m_maxCooldown;
                spawner.m_count -= 1;
            }
        }

        if (spawner.m_groupCount > 0) {
            if (spawner.m_groupCooldown > 0) {
                spawner.m_groupCooldown -= static_cast<int32_t>(ticks);
            }

            if (spawner.m_groupCooldown <= 0) {
                spawner.m_groupCount -= 1;
                spawner.m_groupCooldown += spawner.m_maxGroupCooldown;

                if (spawner.m_groupCount <= 0 && spawner.m_count <= 0) {
                    registry.emplace<DeadTag>(entity);
                }

                auto position = spawnerPosition.m_position;
                if (spawner.m_type == SpawnerType::Staggered) {
                    auto const side = spawner.m_groupCount % 2;
                    position.x += spawner.m_distance * (side == 0 ? 1.0f : -1.0f);
                }

                auto enemy = CreateEnemy(registry, spawner.m_enemyName, gamedata, position,
                                         spawner.m_behaviourName, spawner.m_behaviourParameters);

                if (spawner.m_maxGroupCount > 1) {
                    if (spawner.m_currentGroupEntity == entt::null) {
                        spawner.m_currentGroupEntity = SystemGroup::CreateGroup(registry);
                    }
                    SystemGroup::AddMember(registry, spawner.m_currentGroupEntity, enemy);
                }
            }
        }
    }
}

entt::entity SystemEnemySpawner::CreateEnemy(entt::registry& registry, std::string const& name,
                                             Gamedata const& gamedata, moth_ui::FloatVec2 const& position,
                                             std::string const& behaviourName,
                                             BehaviourParameterList const& behaviourParameters) {
    auto const* enemyDatabase = gamedata.GetEnemyDatabase();
    if (enemyDatabase == nullptr) {
        return entt::null;
    }

    auto const* weaponDatabase = gamedata.GetWeaponDatabase();
    if (weaponDatabase == nullptr) {
        return entt::null;
    }

    auto enemy = registry.create();
    auto& entityData = registry.emplace<ComponentEntity>(enemy);
    auto& health = registry.emplace<ComponentHealth>(enemy);
    auto& drawable = registry.emplace<ComponentDrawable>(enemy);
    auto& pos = registry.emplace<ComponentPosition>(enemy);
    auto& weapon = registry.emplace<ComponentWeapon>(enemy);
    auto& lifetime = registry.emplace<ComponentLifetime>(enemy);
    auto& behaviour = registry.emplace<ComponentBehaviour>(enemy);
    registry.emplace<TargetTag>(enemy);

    auto const* enemyData = enemyDatabase->Get(name);

    entityData.m_team = Team::ENEMY;
    entityData.m_color = EnergyColor::WHITE;
    entityData.m_radius = enemyData->radius;

    health.m_currentHealth = enemyData->health;
    health.m_maxHealth = enemyData->health;

    drawable.m_spriteData = enemyData->sprite;

    pos.m_position = position;

    auto const* weaponData = weaponDatabase->Get(enemyData->weapon_name);
    if (weaponData != nullptr) {
        weapon.m_projectileName = weaponData->projectile_name;
        weapon.m_playerTracking = weaponData->player_tracking;
        weapon.m_maxCooldown = weaponData->cooldown;
        weapon.m_cooldown = weapon.m_maxCooldown;
        weapon.m_angle = entityData.m_team == Team::PLAYER ? 0 : M_PI;
        weapon.m_active = true;
    }

    lifetime.m_msAlive = 0;
    lifetime.m_msLeft = enemyData->lifetime;

    behaviour.m_behaviourName = behaviourName;
    behaviour.m_parameters = behaviourParameters;
    behaviour.m_offset = pos.m_position;

    return enemy;
}

entt::entity SystemEnemySpawner::CreateSpawner(entt::registry& registry, std::string const& name,
                                               Gamedata const& gamedata, moth_ui::FloatVec2 const& position) {
    auto const* spawnerData = gamedata.GetSpawnerDatabase()->Get(name);
    if (spawnerData == nullptr) {
        spdlog::error("Unable to access spawner data");
        return entt::null;
    }
    auto enemySpawner = registry.create();

    auto& spawner = registry.emplace<ComponentEnemySpawner>(enemySpawner);
    spawner.m_active = true;
    spawner.m_type = spawnerData->type;
    spawner.m_distance = spawnerData->distance;
    spawner.m_count = spawnerData->count;
    spawner.m_maxCooldown = spawnerData->cooldown;
    spawner.m_maxGroupCooldown = spawnerData->group_delay;
    spawner.m_maxGroupCount = spawnerData->group_count;
    spawner.m_enemyName = spawnerData->enemy_name;
    spawner.m_behaviourName = spawnerData->behaviour_name;
    spawner.m_behaviourParameters = spawnerData->behaviour_parameters;
    spawner.m_currentGroupEntity = entt::null;

    auto& spawnerPos = registry.emplace<ComponentPosition>(enemySpawner);
    spawnerPos.m_position = position;

    return enemySpawner;
}
