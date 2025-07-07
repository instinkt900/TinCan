#include "system_enemy_spawner.h"
#include "game_layer.h"
#include "system_behaviour.h"
#include "system_projectile.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_drawable.h"
#include "system_weapon.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

void SystemEnemySpawner::Update(entt::registry& registry, uint32_t ticks, Gamedata& databases) {
    auto const* enemyDatabase = databases.GetEnemyDatabase();
    if (enemyDatabase == nullptr) {
        return;
    }

    auto const* weaponDatabase = databases.GetWeaponDatabase();
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

                auto enemy = registry.create();
                auto& entityData = registry.emplace<ComponentEntity>(enemy);
                auto& health = registry.emplace<ComponentHealth>(enemy);
                auto& drawable = registry.emplace<ComponentDrawable>(enemy);
                auto& pos = registry.emplace<ComponentPosition>(enemy);
                auto& weapon = registry.emplace<ComponentWeapon>(enemy);
                auto& lifetime = registry.emplace<ComponentLifetime>(enemy);
                auto& behaviour = registry.emplace<ComponentBehaviour>(enemy);

                auto const* enemyData = enemyDatabase->Get(spawner.m_enemyName);

                entityData.m_team = Team::ENEMY;
                entityData.m_color = EnergyColor::WHITE;
                entityData.m_radius = enemyData->radius;

                health.m_currentHealth = enemyData->health;
                health.m_maxHealth = enemyData->health;
                health.m_onDeath = [&](entt::entity thisEntity, entt::entity killerEntity) {
                    auto& lt = registry.get<ComponentLifetime>(thisEntity);
                    lt.m_msLeft = 0;
                };

                drawable.m_spriteData = enemyData->sprite;

                pos.m_position = spawnerPosition.m_position;

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

                behaviour.m_behaviourName = spawner.m_behaviourName;
                behaviour.m_parameters = spawner.m_behaviourParameters;
                behaviour.m_offset = pos.m_position;
            }
        }
    }
}
