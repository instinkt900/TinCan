#include "system_projectile.h"
#include "collision_utils.h"
#include "component_body.h"
#include "component_drop.h"
#include "component_entity.h"
#include "component_health.h"
#include "gamedata_projectile.h"
#include "system_drawable.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_pickup.h"
#include "system_targeting.h"
#include "system_world_bounds.h"
#include "tags.h"
#include <entt/entt.hpp>
#include <moth_ui/utils/vector_utils.h>
#include <spdlog/spdlog.h>
#include "game_world.h"

namespace {
    float const ComplimentAffinityMult = 2.0f;

    struct Projectile {
        entt::entity entity;
        ComponentEntity& entityDetails;
        ComponentBody& body;
        ComponentProjectile& projectileDetails;
        ComponentPosition& position;
    };

    struct Target {
        entt::entity entity;
        ComponentEntity const& entityDetails;
        ComponentBody const& body;
        ComponentPosition const& position;
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
            if (CollisionTest(projectile.position, target.position, projectile.body.m_radius,
                              target.body.m_radius)) {
                // projectile collided with entity

                // destroy projectile
                registry.get_or_emplace<DeadTag>(projectile.entity);

                // apply damage
                if (auto* entityHealth = registry.try_get<ComponentHealth>(target.entity)) {
                    auto damage = projectile.projectileDetails.m_damage;
                    if (projectile.entityDetails.m_affinity != target.entityDetails.m_affinity) {
                        damage *= ComplimentAffinityMult;
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

    ComponentEntity const* GetEntityDetails(entt::registry& registry, entt::entity entity) {
        if (auto const* entityDetails = registry.try_get<ComponentEntity>(entity)) {
            return entityDetails;
        }

        if (auto const* parenting = registry.try_get<ComponentParenting>(entity)) {
            return GetEntityDetails(registry, parenting->m_parent);
        }

        return nullptr;
    }

    void ProcessProjectile(entt::registry& registry, Projectile& projectile, GameData const& gamedata) {
        auto view = registry.view<ComponentBody, ComponentPosition, TargetTag>(entt::exclude<DeadTag>);
        for (auto [entity, body, position] : view.each()) {
            if (projectile.entity == entity) {
                // shouldnt happen but here for safety
                continue;
            }
            auto const* entityDetails = GetEntityDetails(registry, entity);
            Target target{ entity, *entityDetails, body, position };
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
    registry.emplace<ComponentBody>(projectile, data.radius);
    registry.emplace<ComponentPosition>(projectile, position);
    auto& projectileVel = registry.emplace<ComponentVelocity>(projectile);

    projectileComp.m_source = source;
    projectileComp.m_damage = damage;

    projectileEntityData.m_team = sourceEntityData->m_team;
    projectileEntityData.m_affinity = sourceEntityData->m_affinity;
    projectileEntityData.m_angle = rotation;

    projectileVel.m_velocity = direction * -data.speed; // -speed because we want -y as up

    auto const& spriteData =
        sourceEntityData->m_affinity == Affinity::Light ? data.white_sprite : data.black_sprite;
    auto& sprite = registry.emplace<ComponentSprite>(projectile, *spriteData);
    sprite.m_color = sourceEntityData->m_affinity == Affinity::Light ? canyon::graphics::BasicColors::Blue
                                                                     : canyon::graphics::BasicColors::Red;

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
    auto view = registry.view<ComponentEntity, ComponentProjectile, ComponentBody, ComponentPosition>(
        entt::exclude<DeadTag>);
    for (auto [entity, entityDetails, projectileDetails, body, position] : view.each()) {
        Projectile projectile{ entity, entityDetails, body, projectileDetails, position };
        ProcessProjectile(registry, projectile, gamedata);
    }
}
