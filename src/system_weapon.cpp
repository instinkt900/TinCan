#include "system_weapon.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_sprite.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

void SystemWeapon::Update(entt::registry& registry, uint32_t ticks) {
    auto view = registry.view<ComponentWeapon, ComponentPosition>();
    for (auto [entity, weapon, position] : view.each()) {
        if (weapon.m_cooldown <= ticks) {
            if (weapon.m_active) {
                auto projectile = registry.create();
                auto& projectileData = registry.emplace<ComponentProjectile>(projectile);
                auto& projectilePos = registry.emplace<ComponentPosition>(projectile);
                auto& projectileVel = registry.emplace<ComponentVelocity>(projectile);
                auto& projectileSprite = registry.emplace<ComponentSprite>(projectile);
                auto& projectileLifetime = registry.emplace<ComponentLifetime>(projectile);

                projectileData.m_owner = entity;
                projectileData.m_team = weapon.m_projectileTemplate.m_team;
                projectileData.m_color = weapon.m_projectileTemplate.m_color;
                projectileData.m_damage = weapon.m_projectileTemplate.m_damage;
                projectileData.m_radius = weapon.m_projectileTemplate.m_radius;
                projectilePos.m_position = position.m_position;
                projectileVel.m_velocity = { 0, weapon.m_projectileTemplate.m_speed };
                projectileVel.m_velocity.y *= weapon.m_projectileTemplate.m_team == Team::PLAYER ? -1 : 1;
                projectileSprite.m_sprite = weapon.m_projectileTemplate.m_sprite;
                projectileSprite.m_size = weapon.m_projectileTemplate.m_spriteSize;
                projectileLifetime.m_lifetime = weapon.m_projectileTemplate.m_lifetime;

                weapon.m_cooldown = weapon.m_maxCooldown - (ticks - weapon.m_cooldown);
            }
        } else {
            weapon.m_cooldown -= ticks;
        }
    }
}
