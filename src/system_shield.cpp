#include "system_shield.h"
#include "collision_utils.h"
#include "game_layer.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include <entt/entt.hpp>

void SystemShield::Update(entt::registry& registry, uint32_t ticks) {
    auto view = registry.view<ComponentShield, ComponentEntity, ComponentPosition, ComponentVelocity>();

    for (auto [shieldEntity, shieldData, shieldEntityData, shieldPosition, shieldVelocity] : view.each()) {
        auto projectileView =
            registry.view<ComponentProjectile, ComponentEntity, ComponentPosition, ComponentVelocity>();

        for (auto [projectileEntity, projectileData, projectileEntityData, projectilePosition,
                   projectileVelocity] : projectileView.each()) {
            if (projectileData.m_color == shieldEntityData.m_color &&
                projectileEntityData.m_team != shieldEntityData.m_team) {

                auto const t = SweepTest(shieldPosition.m_position, shieldPosition.m_lastPosition,
                                         shieldEntityData.m_radius, projectilePosition.m_position,
                                         projectilePosition.m_lastPosition, projectileEntityData.m_radius);
                if (t < 1.0f) {
                    // projectile collided with shield

                    // destroy projectile
                    if (auto* projectileLifetime = registry.try_get<ComponentLifetime>(projectileEntity)) {
                        projectileLifetime->m_msLeft = 0;
                    }

                    // apply power
                    if (auto* ownerPower = registry.try_get<ComponentPower>(shieldEntity)) {
                        ownerPower->m_power += projectileData.m_damage;
                    }
                }
            }
        }
    }
}
