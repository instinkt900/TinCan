#include "system_projectile.h"
#include "collision_utils.h"
#include "game_layer.h"
#include "system_health.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_pickup.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <moth_ui/utils/vector_utils.h>
#include <spdlog/spdlog.h>

namespace {
    struct Projectile {
        entt::entity entity;
        ComponentEntity& entityDetails;
        ComponentProjectile& projectileDetails;
        ComponentPosition& position;
    };

    struct Target {
        entt::entity entity;
        ComponentEntity& entityDetails;
        ComponentPosition& position;
    };

    bool CollisionTest(ComponentPosition const& pos1, ComponentPosition const& pos2, float radius1,
                       float radius2) {
        auto const entity1Vel = pos1.m_position - pos1.m_lastPosition;
        auto const entity2Vel = pos2.m_position - pos2.m_lastPosition;
        auto const relativeVel = entity2Vel - entity1Vel;
        auto const combinedRadius = radius1 + radius2;
        auto const t = SweepTest(pos1.m_position, pos2.m_lastPosition, relativeVel, combinedRadius);
        return t < 1.0f;
    }

    void ProcessTarget(entt::registry& registry, Projectile& projectile, Target& target, Gamedata const& gamedata) {
        if (projectile.entityDetails.m_team != target.entityDetails.m_team) {
            if (CollisionTest(projectile.position, target.position, projectile.entityDetails.m_radius,
                              target.entityDetails.m_radius)) {
                // projectile collided with entity

                // destroy projectile
                registry.get_or_emplace<DeadTag>(projectile.entity);

                // apply damage
                if (auto* entityHealth = registry.try_get<ComponentHealth>(target.entity)) {
                    if (entityHealth->m_currentHealth > 0) {
                        entityHealth->m_currentHealth -= projectile.projectileDetails.m_damage;
                        if (entityHealth->m_currentHealth <= 0) {
                            registry.emplace<DeadTag>(target.entity);

                            if (auto const* dropComponent = registry.try_get<ComponentDrop>(target.entity)) {
                                SystemPickup::CreatePickup(registry, target.position.m_position, dropComponent->m_dropName, gamedata);
                            }
                        }
                    }
                }
            }
        }
    }

    void ProcessProjectile(entt::registry& registry, Projectile& projectile, Gamedata const& gamedata) {
        auto view = registry.view<ComponentEntity, ComponentPosition, TargetTag>();
        for (auto [entity, entityDetails, position] : view.each()) {
            if (projectile.entity == entity) {
                // shouldnt happen but here for safety
                continue;
            }
            Target target{ entity, entityDetails, position };
            ProcessTarget(registry, projectile, target, gamedata);
        }
    }
}

void SystemProjectile::Update(entt::registry& registry, uint32_t ticks, Gamedata const& gamedata) {
    auto view = registry.view<ComponentEntity, ComponentProjectile, ComponentPosition>();
    for (auto [entity, entityDetails, projectileDetails, position] : view.each()) {
        Projectile projectile{ entity, entityDetails, projectileDetails, position };
        ProcessProjectile(registry, projectile, gamedata);
    }
}
