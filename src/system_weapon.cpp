#include "system_weapon.h"
#include "game_layer.h"
#include "system_movement.h"
#include <entt/entt.hpp>
#include <moth_ui/utils/vector_utils.h>
#include <spdlog/spdlog.h>

void SystemWeapon::Update(entt::registry& registry, uint32_t ticks, ProjectileDatabase& projectileDatabase) {
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

            projectileDatabase.SpawnProjectile(weapon.m_projectileName, registry, entity, position.m_position,
                                               direction, entityData.m_color, entityData.m_team, 3000);

            weapon.m_cooldown += static_cast<int32_t>(weapon.m_maxCooldown);
        }
    }
}
