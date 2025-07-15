#include "system_shield.h"
#include "collision_utils.h"
#include "component_entity.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_projectile.h"
#include <entt/entt.hpp>
#include "game_world.h"
#include "tags.h"

void SystemShield::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto view = registry.view<ComponentShield, ComponentEntity, ComponentPosition>();

    for (auto [shieldEntity, shieldData, shieldEntityData, shieldPosition] : view.each()) {
        auto projectileView = registry.view<ComponentProjectile, ComponentEntity, ComponentPosition>();

        for (auto [projectileEntity, projectileData, projectileEntityData, projectilePosition] :
             projectileView.each()) {
            if (projectileData.m_color == shieldEntityData.m_color &&
                projectileEntityData.m_team != shieldEntityData.m_team) {

                auto const t = SweepTest(shieldPosition.m_position, shieldPosition.m_lastPosition,
                                         shieldData.m_radius, projectilePosition.m_position,
                                         projectilePosition.m_lastPosition, projectileEntityData.m_radius);
                if (t < 1.0f) {
                    // projectile collided with shield
                    spdlog::info("projectile hit shield");

                    // destroy projectile
                    registry.get_or_emplace<DeadTag>(projectileEntity);

                    // apply power
                    if (auto* ownerPower = registry.try_get<ComponentPower>(shieldEntity)) {
                        ownerPower->m_power += projectileData.m_damage;
                    }
                }
            }
        }
    }
}
