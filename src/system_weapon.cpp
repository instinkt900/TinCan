#include "system_weapon.h"
#include "game_layer.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_drawable.h"
#include <entt/entt.hpp>
#include <moth_ui/utils/vector_utils.h>
#include <spdlog/spdlog.h>
#include "utils.h"

void SystemWeapon::Update(entt::registry& registry, uint32_t ticks) {
    ComponentPosition* playerPosition = nullptr;

    auto playerView = registry.view<ComponentPosition, PlayerTag>();
    if (playerView.begin() != playerView.end()) {
        auto playerEntity = *playerView.begin();
        playerPosition = registry.try_get<ComponentPosition>(playerEntity);
    }
    auto view = registry.view<ComponentWeapon, ComponentEntity, ComponentPosition>();
    for (auto [entity, weapon, entityData, position] : view.each()) {
        if (weapon.m_cooldown <= ticks) {
            while (weapon.m_active && weapon.m_cooldown <= ticks) {
                auto projectile = registry.create();
                auto& projectileData = registry.emplace<ComponentProjectile>(projectile);
                auto& projectileEntityData = registry.emplace<ComponentEntity>(projectile);
                auto& projectilePos = registry.emplace<ComponentPosition>(projectile);
                auto& projectileVel = registry.emplace<ComponentVelocity>(projectile);
                auto& projectileDrawable = registry.emplace<ComponentDrawable>(projectile);
                auto& projectileLifetime = registry.emplace<ComponentLifetime>(projectile);

                projectileData.m_owner = entity;
                projectileData.m_color = entityData.m_color;
                projectileData.m_damage = weapon.m_projectileTemplate.m_damage;
                projectileEntityData.m_team = weapon.m_projectileTemplate.m_team;
                projectileEntityData.m_radius = weapon.m_projectileTemplate.m_radius;
                projectilePos.m_position = position.m_position;
                projectileVel.m_velocity = { 0, weapon.m_projectileTemplate.m_speed };
                projectileVel.m_velocity.y *= weapon.m_projectileTemplate.m_team == Team::PLAYER ? -1 : 1;
                projectileDrawable.m_sprites = weapon.m_projectileTemplate.m_drawable.m_sprites;
                for (auto& sprite: projectileDrawable.m_sprites) {
                    sprite.m_color = GetEnergyColor(entityData.m_color);
                }
                projectileLifetime.m_lifetime = weapon.m_projectileTemplate.m_lifetime;

                if (weapon.m_playerTracking && (playerPosition != nullptr)) {
                    auto delta = playerPosition->m_position - position.m_position;
                    auto velocity = moth_ui::Normalized(delta) * weapon.m_projectileTemplate.m_speed;
                    projectileVel.m_velocity = velocity;
                }

                weapon.m_cooldown += weapon.m_maxCooldown;
            }
        } else {
            weapon.m_cooldown -= ticks;
        }
    }
}
