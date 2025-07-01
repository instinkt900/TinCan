#include "system_weapon.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_sprite.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

void SystemWeapon::Update(entt::registry& registry, uint32_t ticks, canyon::graphics::SurfaceContext& surfaceContext) {
    auto view = registry.view<ComponentWeapon, ComponentPosition>();
    for (auto [entity, weapon, position] : view.each()) {
        if (weapon.m_active) {
            if (weapon.m_cooldown <= ticks) {
                spdlog::info("spawning bullet");
                auto bullet = registry.create();
                auto& bulletPos = registry.emplace<ComponentPosition>(bullet);
                auto& bulletVel = registry.emplace<ComponentVelocity>(bullet);
                auto& bulletData = registry.emplace<ComponentBullet>(bullet);
                auto& bulletSprite = registry.emplace<ComponentSprite>(bullet);
                auto& bulletLifetime = registry.emplace<ComponentLifetime>(bullet);

                bulletPos.m_position = position.m_position;
                bulletVel.m_velocity = {0, -500};
                bulletData.m_player = weapon.m_player;
                bulletData.m_color = weapon.m_color;
                bulletLifetime.m_lifetime = 5000;
                bulletSprite.m_sprite = surfaceContext.ImageFromFile("assets/bullet.png");
                bulletSprite.m_size = {100,100};

                weapon.m_cooldown = weapon.m_maxCooldown - (ticks - weapon.m_cooldown);
            } else {
                weapon.m_cooldown -= ticks;
            }
        }
    }
}
