#include "system_projectile.h"
#include "collision_utils.h"
#include "component_drop.h"
#include "component_entity.h"
#include "component_health.h"
#include "gamedata_projectile.h"
#include "system_drawable.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_pickup.h"
#include "system_targeting.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <moth_ui/utils/vector_utils.h>
#include <spdlog/spdlog.h>
#include "game_world.h"

namespace {
    float const OffColorMult = 2.0f;

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
        auto const t = SweepTest(pos1.m_position, pos1.m_lastPosition, radius1, pos2.m_position,
                                 pos2.m_lastPosition, radius2);
        return t < 1.0f;
    }

    void ProcessTarget(entt::registry& registry, Projectile& projectile, Target& target,
                       GameData const& gamedata) {
        if (projectile.entityDetails.m_team != target.entityDetails.m_team) {
            if (CollisionTest(projectile.position, target.position, projectile.entityDetails.m_radius,
                              target.entityDetails.m_radius)) {
                // projectile collided with entity

                // destroy projectile
                registry.get_or_emplace<DeadTag>(projectile.entity);

                // apply damage
                if (auto* entityHealth = registry.try_get<ComponentHealth>(target.entity)) {
                    auto damage = projectile.projectileDetails.m_damage;
                    if (projectile.projectileDetails.m_color != target.entityDetails.m_color) {
                        damage *= OffColorMult;
                    }
                    if (entityHealth->m_currentHealth > 0) {
                        entityHealth->m_currentHealth -= damage;
                        if (entityHealth->m_currentHealth <= 0) {
                            SystemDrawable::StartCameraShake(registry, 0.5f, 500);
                            registry.emplace<DeadTag>(target.entity);

                            if (auto const* dropComponent = registry.try_get<ComponentDrop>(target.entity)) {
                                auto const* pickupData = gamedata.Get<PickupData>(dropComponent->m_name);
                                if (pickupData == nullptr) {
                                    spdlog::error("Unknown pickup {}", dropComponent->m_name);
                                    return;
                                }
                                SystemPickup::CreatePickup(registry, target.position.m_position, *pickupData,
                                                           gamedata);
                            }
                        }
                    }
                }
            }
        }
    }

    void ProcessProjectile(entt::registry& registry, Projectile& projectile, GameData const& gamedata) {
        auto view = registry.view<ComponentEntity, ComponentPosition, TargetTag>(entt::exclude<DeadTag>);
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

entt::entity SystemProjectile::CreateProjectile(entt::registry& registry, ProjectileData const& data,
                                                entt::entity source, canyon::FloatVec2 const& position,
                                                canyon::FloatVec2 const& direction, float rotation,
                                                float damage) {
    auto const* sourceEntityData = registry.try_get<ComponentEntity>(source);
    if (sourceEntityData == nullptr) {
        return entt::null;
    }
    auto projectile = registry.create();
    auto& projectileComp = registry.emplace<ComponentProjectile>(projectile);
    auto& projectileEntityData = registry.emplace<ComponentEntity>(projectile);
    auto& projectilePos = registry.emplace<ComponentPosition>(projectile);
    auto& projectileVel = registry.emplace<ComponentVelocity>(projectile);

    projectileComp.m_source = source;
    projectileComp.m_color = sourceEntityData->m_color;
    projectileComp.m_damage = damage;

    projectileEntityData.m_team = sourceEntityData->m_team;
    projectileEntityData.m_radius = data.radius;
    projectileEntityData.m_angle = rotation;
    projectilePos.m_position = position;
    projectileVel.m_velocity = direction * -data.speed; // -speed because we want -y as up

    auto const& sprite =
        sourceEntityData->m_color == EnergyColor::Blue ? data.white_sprite : data.black_sprite;
    registry.emplace<ComponentDrawable>(projectile, *sprite);

    if (data.homing_details.has_value()) {
        if (data.homing_details->targeting != Targeting::None) {
            auto& targeting = registry.emplace<ComponentTargeting>(projectile);
            targeting.m_targeting = data.homing_details->targeting;
            targeting.m_maxTurnSpeed = data.homing_details->turn_speed;
            targeting.m_maxThrust = data.homing_details->max_thrust;
            targeting.m_maxSpeed = data.homing_details->max_speed;
            targeting.m_minDrag = data.homing_details->min_drag;
            targeting.m_maxDrag = data.homing_details->max_drag;
        }
    }

    return projectile;
}

void SystemProjectile::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto const& gamedata = world.GetGameData();
    auto view =
        registry.view<ComponentEntity, ComponentProjectile, ComponentPosition>(entt::exclude<DeadTag>);
    for (auto [entity, entityDetails, projectileDetails, position] : view.each()) {
        Projectile projectile{ entity, entityDetails, projectileDetails, position };
        ProcessProjectile(registry, projectile, gamedata);
    }
}
