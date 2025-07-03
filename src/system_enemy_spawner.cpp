#include "system_enemy_spawner.h"
#include "system_collision.h"
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
            auto& enemyComponent = registry.emplace<ComponentEnemy>(enemy);
            auto& collision = registry.emplace<ComponentCollision>(enemy);
            auto& sprite = registry.emplace<ComponentSprite>(enemy);
            auto& pos = registry.emplace<ComponentPosition>(enemy);
            auto& vel = registry.emplace<ComponentVelocity>(enemy);
            auto& weapon = registry.emplace<ComponentWeapon>(enemy);
            auto& lifetime = registry.emplace<ComponentLifetime>(enemy);

            enemyComponent.m_health = 100;

            collision.m_radius = 5;
            collision.m_collisionMask = CollisionGroups::ENEMY;
            collision.m_collidesWithMask = 0;

            sprite.m_sprite = spawner.m_template.m_sprite;
            sprite.m_size = spawner.m_template.m_spriteSize;

            float const xfact = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
            float const yfact = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
            float const minX = static_cast<float>(spawner.m_spawnRegion.x());
            float const minY = static_cast<float>(spawner.m_spawnRegion.y());
            float const width = static_cast<float>(spawner.m_spawnRegion.w()) * xfact;
            float const height = static_cast<float>(spawner.m_spawnRegion.h()) * yfact;
            pos.m_position.x = minX + width;
            pos.m_position.y = minY + height;

            vel.m_velocity.x = 0;
            vel.m_velocity.y = spawner.m_template.m_speed;

            weapon.m_player = false;
            weapon.m_active = true;
            weapon.m_maxCooldown = spawner.m_template.m_weapon.m_maxCooldown;
            weapon.m_cooldown = weapon.m_maxCooldown;
            weapon.m_projectile.m_sprite = spawner.m_template.m_weapon.m_projectile.m_sprite;
            weapon.m_projectile.m_spriteSize = spawner.m_template.m_weapon.m_projectile.m_spriteSize;
            weapon.m_projectile.m_speed = spawner.m_template.m_weapon.m_projectile.m_speed;
            weapon.m_projectile.m_lifetime = spawner.m_template.m_weapon.m_projectile.m_lifetime;
            weapon.m_projectile.m_collisionMask = spawner.m_template.m_weapon.m_projectile.m_collisionMask;
            weapon.m_projectile.m_collidesWithMask = spawner.m_template.m_weapon.m_projectile.m_collidesWithMask;
            weapon.m_projectile.m_radius = spawner.m_template.m_weapon.m_projectile.m_radius;
            weapon.m_projectile.m_onCollision = spawner.m_template.m_weapon.m_projectile.m_onCollision;

            lifetime.m_lifetime = 5000;

            spawner.m_cooldown += spawner.m_maxCooldown - (ticks - spawner.m_cooldown);
        }
    }
}
