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

void SystemEnemySpawner::Update(entt::registry& registry, uint32_t ticks,
                                SpriteDatabase const& spriteDatabase, EnemyDatabase const& enemyDatabase,
                                WeaponDatabase const& weaponDatabase) {
    auto view = registry.view<ComponentEnemySpawner, ComponentPosition>();

    for (auto [entity, spawner, spawnerPosition] : view.each()) {
        if (spawner.m_cooldown > 0) {
            spawner.m_cooldown -= static_cast<int32_t>(ticks);
        }

        if (spawner.m_burstDelay > 0) {
            spawner.m_burstDelay -= static_cast<int32_t>(ticks);
        }

        if (spawner.m_active && spawner.m_cooldown <= 0) {
            spawner.m_burst = spawner.m_maxBurst;
            spawner.m_cooldown += spawner.m_maxCooldown;
        }

        if (spawner.m_burst > 0 && spawner.m_burstDelay <= 0) {
            spawner.m_burst -= 1;
            spawner.m_burstDelay += spawner.m_maxBurstDelay;

            auto enemy = registry.create();
            auto& entityData = registry.emplace<ComponentEntity>(enemy);
            auto& health = registry.emplace<ComponentHealth>(enemy);
            auto& drawable = registry.emplace<ComponentDrawable>(enemy);
            auto& pos = registry.emplace<ComponentPosition>(enemy);
            auto& weapon = registry.emplace<ComponentWeapon>(enemy);
            auto& lifetime = registry.emplace<ComponentLifetime>(enemy);
            auto& behaviour = registry.emplace<ComponentBehaviour>(enemy);

            auto const* enemyData = enemyDatabase.GetEnemyData(spawner.m_enemyName);

            entityData.m_team = Team::ENEMY;
            entityData.m_color = EnergyColor::WHITE;
            entityData.m_radius = enemyData->radius;

            health.m_currentHealth = enemyData->health;
            health.m_maxHealth = enemyData->health;
            health.m_onDeath = [&](entt::entity thisEntity, entt::entity killerEntity) {
                auto& lt = registry.get<ComponentLifetime>(thisEntity);
                lt.m_msLeft = 0;
            };

            auto const* sprite = spriteDatabase.GetSpriteData(enemyData->sprite_name);
            if (sprite != nullptr) {
                drawable.m_spriteData = *sprite;
            }

            // float const xfact = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
            // float const yfact = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
            // float const minX = static_cast<float>(spawner.m_spawnRegion.x());
            // float const minY = static_cast<float>(spawner.m_spawnRegion.y());
            // float const width = static_cast<float>(spawner.m_spawnRegion.w()) * xfact;
            // float const height = static_cast<float>(spawner.m_spawnRegion.h()) * yfact;
            // pos.m_position.x = minX + width;
            // pos.m_position.y = minY + height;
            pos.m_position = spawnerPosition.m_position;

            auto const* weaponData = weaponDatabase.GetWeaponData(enemyData->weapon_name);
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

            behaviour.m_behaviourName = enemyData->behaviour_name;
            behaviour.m_parameters = enemyData->behaviour_parameters;
            behaviour.m_offset = pos.m_position;
        }
    }
}
