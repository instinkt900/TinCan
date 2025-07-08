#include "system_weapon.h"
#include "game_layer.h"
#include "system_drawable.h"
#include "system_movement.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <moth_ui/utils/vector_utils.h>
#include <spdlog/spdlog.h>

void SystemWeapon::Update(entt::registry& registry, uint32_t ticks, Gamedata const& databases) {
    auto const* projectileDatabase = databases.GetProjectileDatabase();
    if (projectileDatabase == nullptr) {
        return;
    }

    ComponentPosition* playerPosition = nullptr;
    auto playerView = registry.view<ComponentPosition, PlayerTag>();
    if (playerView.begin() != playerView.end()) {
        auto playerEntity = *playerView.begin();
        playerPosition = registry.try_get<ComponentPosition>(playerEntity);
    }

    auto view = registry.view<ComponentWeapon, ComponentEntity, ComponentPosition>();
    for (auto [entity, weapon, entityData, position] : view.each()) {
        if (weapon.m_cooldown > 0) {
            weapon.m_cooldown -= static_cast<int32_t>(ticks);
        }
        while (weapon.m_active && weapon.m_cooldown <= 0) {

            auto direction = moth_ui::Rotate2D({ 0, 1.0f }, weapon.m_angle);
            if (weapon.m_playerTracking && (playerPosition != nullptr)) {
                auto delta = playerPosition->m_position - position.m_position;
                direction = moth_ui::Normalized(delta);
            }

            auto const* projectileData = projectileDatabase->Get(weapon.m_projectileName);
            if (projectileData != nullptr) {

                auto projectile = registry.create();
                auto& projectileComp = registry.emplace<ComponentProjectile>(projectile);
                auto& projectileEntityData = registry.emplace<ComponentEntity>(projectile);
                auto& projectilePos = registry.emplace<ComponentPosition>(projectile);
                auto& projectileVel = registry.emplace<ComponentVelocity>(projectile);
                auto& projectileDrawable = registry.emplace<ComponentDrawable>(projectile);
                auto& projectileLifetime = registry.emplace<ComponentLifetime>(projectile);

                projectileComp.m_owner = entity;
                projectileComp.m_color = entityData.m_color;
                projectileComp.m_damage = projectileData->damage;

                projectileEntityData.m_team = entityData.m_team;
                projectileEntityData.m_radius = projectileData->radius;
                projectilePos.m_position = position.m_position;
                projectileVel.m_velocity = direction * projectileData->speed;

                auto const& sprite = entityData.m_color == EnergyColor::WHITE ? projectileData->white_sprite
                                                                              : projectileData->black_sprite;
                projectileDrawable.m_spriteData = sprite;
                projectileLifetime.m_msLeft = 3000;
            }

            weapon.m_cooldown += weapon.m_maxCooldown;
        }
    }
}
