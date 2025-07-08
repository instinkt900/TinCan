#include "system_projectile.h"
#include "collision_utils.h"
#include "game_layer.h"
#include "system_health.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <moth_ui/utils/vector_utils.h>
#include <spdlog/spdlog.h>

void SystemProjectile::Update(entt::registry& registry, uint32_t ticks) {
    auto projectileView =
        registry.view<ComponentEntity, ComponentProjectile, ComponentPosition, ComponentVelocity>();
    for (auto [projectileEntity, projectileEntityData, projectileData, projectilePosition,
               projectileVelocity] : projectileView.each()) {
        auto entityView = registry.view<ComponentEntity, ComponentPosition>(
            entt::exclude<ComponentProjectile>);
        for (auto [entity, entityData, entityPosition] : entityView.each()) {
            if (projectileEntity == entity) {
                // shouldnt happen but here for safety
                continue;
            }
            if (projectileEntityData.m_team != entityData.m_team) {
                auto const entity1Vel = projectilePosition.m_position - projectilePosition.m_lastPosition;
                auto const entity2Vel = entityPosition.m_position - entityPosition.m_lastPosition;
                auto const relativeVel = entity2Vel - entity1Vel;
                auto const combinedRadius = projectileEntityData.m_radius + entityData.m_radius;
                auto const t = SweepTest(projectilePosition.m_position, entityPosition.m_lastPosition,
                                         relativeVel, combinedRadius);
                if (t < 1.0f) {
                    // projectile collided with entity

                    // destroy projectile
                    if (auto* projectileLifetime = registry.try_get<ComponentLifetime>(projectileEntity)) {
                        projectileLifetime->m_msLeft = 0;
                    }

                    // apply damage
                    if (auto* entityHealth = registry.try_get<ComponentHealth>(entity)) {
                        if (entityHealth->m_currentHealth > 0) {
                            entityHealth->m_currentHealth -= projectileData.m_damage;
                            if (entityHealth->m_currentHealth <= 0) {
                                registry.emplace<DeadTag>(entity);
                            }
                        }
                    }
                }
            }
        }
    }
}
