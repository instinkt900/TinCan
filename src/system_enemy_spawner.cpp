#include "system_enemy_spawner.h"
#include "game_layer.h"
#include "system_projectile.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_sprite.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

void SystemEnemySpawner::Update(entt::registry& registry, uint32_t ticks) {
    auto view = registry.view<ComponentEnemySpawner>();

    for (auto [entity, spawner] : view.each()) {
        if (spawner.m_cooldown > ticks) {
            spawner.m_cooldown -= ticks;
        }

        if (spawner.m_active && spawner.m_cooldown <= ticks) {
            auto enemy = registry.create();
            auto& entityData = registry.emplace<ComponentEntity>(enemy);
            auto& health = registry.emplace<ComponentHealth>(enemy);
            auto& sprite = registry.emplace<ComponentSprite>(enemy);
            auto& pos = registry.emplace<ComponentPosition>(enemy);
            auto& vel = registry.emplace<ComponentVelocity>(enemy);
            auto& weapon = registry.emplace<ComponentWeapon>(enemy);
            auto& lifetime = registry.emplace<ComponentLifetime>(enemy);

            entityData.m_team = spawner.m_enemyTemplate.m_team;
            entityData.m_radius = spawner.m_enemyTemplate.m_radius;

            health.m_currentHealth = spawner.m_enemyTemplate.m_maxHealth;
            health.m_maxHealth = spawner.m_enemyTemplate.m_maxHealth;
            health.m_onDeath = spawner.m_enemyTemplate.m_onDeath;

            sprite.m_sprite = spawner.m_enemyTemplate.m_sprite;
            sprite.m_size = spawner.m_enemyTemplate.m_spriteSize;

            float const xfact = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
            float const yfact = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
            float const minX = static_cast<float>(spawner.m_spawnRegion.x());
            float const minY = static_cast<float>(spawner.m_spawnRegion.y());
            float const width = static_cast<float>(spawner.m_spawnRegion.w()) * xfact;
            float const height = static_cast<float>(spawner.m_spawnRegion.h()) * yfact;
            pos.m_position.x = minX + width;
            pos.m_position.y = minY + height;

            vel.m_velocity.x = 0;
            vel.m_velocity.y = spawner.m_enemyTemplate.m_speed;

            weapon.m_active = true;
            weapon.m_cooldown = spawner.m_enemyTemplate.m_weapon.m_maxCooldown;
            weapon.m_maxCooldown = spawner.m_enemyTemplate.m_weapon.m_maxCooldown;
            weapon.m_projectileTemplate.m_sprite = spawner.m_enemyTemplate.m_weapon.m_projectileTemplate.m_sprite;
            weapon.m_projectileTemplate.m_spriteSize = spawner.m_enemyTemplate.m_weapon.m_projectileTemplate.m_spriteSize;
            weapon.m_projectileTemplate.m_team = spawner.m_enemyTemplate.m_weapon.m_projectileTemplate.m_team;
            weapon.m_projectileTemplate.m_damage = spawner.m_enemyTemplate.m_weapon.m_projectileTemplate.m_damage;
            weapon.m_projectileTemplate.m_speed = spawner.m_enemyTemplate.m_weapon.m_projectileTemplate.m_speed;
            weapon.m_projectileTemplate.m_lifetime = spawner.m_enemyTemplate.m_weapon.m_projectileTemplate.m_lifetime;
            weapon.m_projectileTemplate.m_radius = spawner.m_enemyTemplate.m_weapon.m_projectileTemplate.m_radius;

            lifetime.m_lifetime = spawner.m_enemyTemplate.m_lifetime;;

            spawner.m_cooldown += spawner.m_maxCooldown - (ticks - spawner.m_cooldown);
        }
    }
}
