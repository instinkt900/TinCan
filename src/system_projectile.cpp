#include "system_projectile.h"
#include "game_layer.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include <entt/entt.hpp>
#include <moth_ui/utils/vector_utils.h>
#include <spdlog/spdlog.h>

float Sweep(moth_ui::FloatVec2 const& stationary, moth_ui::FloatVec2 const& moving,
            moth_ui::FloatVec2 const& velocity, float radius) {
    auto const delta = moving - stationary;

    float const a = moth_ui::Dot(velocity, velocity);
    float const b = 2.0f * moth_ui::Dot(delta, velocity);
    float const c = moth_ui::Dot(delta, delta) - (radius * radius);

    if (a == 0.0f) {
        if (moth_ui::DistanceSq(stationary, moving) <= (radius * radius)) {
            return 0.0f;
        }
        return 1.0f;
    }

    float const disc = (b * b) - (4 * a * c);
    if (disc < 0.0f) {
        return 1.0f;
    }

    float const sqrtDist = std::sqrt(disc);
    float const t1 = (-b - sqrtDist) / (2.0f * a);
    float const t2 = (-b + sqrtDist) / (2.0f * a);

    if (t1 >= 0.0f && t1 <= 1.0f) {
        return t1;
    }

    if (t2 >= 0.0f && t2 <= 1.0f) {
        return t2;
    }

    return 1.0f;
}

void SystemProjectile::Update(entt::registry& registry, uint32_t ticks) {
    auto projectileView =
        registry.view<ComponentEntity, ComponentProjectile, ComponentPosition, ComponentVelocity>();
    for (auto [projectileEntity, projectileEntityData, projectileData, projectilePosition,
               projectileVelocity] : projectileView.each()) {
        auto entityView = registry.view<ComponentEntity, ComponentPosition, ComponentVelocity>(
            entt::exclude<ComponentProjectile>);
        for (auto [entity, entityData, entityPosition, entityVelocity] : entityView.each()) {
            if (projectileEntity == entity) {
                // shouldnt happen but here for safety
                continue;
            }
            if (projectileEntityData.m_team != entityData.m_team) {
                auto const entity1Vel = projectilePosition.m_position - projectilePosition.m_lastPosition;
                auto const entity2Vel = entityPosition.m_position - entityPosition.m_lastPosition;
                auto const relativeVel = entity2Vel - entity1Vel;
                auto const combinedRadius = projectileEntityData.m_radius + entityData.m_radius;
                auto const t = Sweep(projectilePosition.m_position, entityPosition.m_lastPosition,
                                     relativeVel, combinedRadius);
                if (t < 1.0f) {
                    // projectile collided with entity

                    // destroy projectile
                    if (auto* projectileLifetime = registry.try_get<ComponentLifetime>(projectileEntity)) {
                        projectileLifetime->m_lifetime = 0;
                    }

                    // apply damage
                    if (auto* entityHealth = registry.try_get<ComponentHealth>(entity)) {
                        if (entityHealth->m_currentHealth > 0) {
                            entityHealth->m_currentHealth -= projectileData.m_damage;
                            if (entityHealth->m_currentHealth <= 0) {
                                if (entityHealth->m_onDeath) {
                                    entityHealth->m_onDeath(entity, projectileData.m_owner);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
