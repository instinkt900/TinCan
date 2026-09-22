#include "system_projectile.h"
#include "utils_collision.h"
#include "component_body.h"
#include "component_entity.h"
#include "gamedata_projectile.h"
#include "system_damage.h"
#include "system_drawable.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_targeting.h"
#include "system_world_bounds.h"
#include "tags.h"
#include <moth/ecs/world.h>
#include <moth/ui/utils/vector_utils.h>
#include "game_world.h"
#include "utils_entity.h"

namespace {
    float const ComplimentAffinityMult = 2.0f;

    struct Projectile {
        moth::ecs::Entity entity;
        ComponentEntity& entityDetails;
        ComponentBody& body;
        ComponentProjectile& projectileDetails;
        ComponentPosition& position;
    };

    struct Target {
        moth::ecs::Entity entity;
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

    void ProcessTarget(moth::ecs::World& registry, Projectile& projectile, Target& target,
                       GameData const& gamedata) {
        if (projectile.entityDetails.m_team != target.entityDetails.m_team) {
            if (CollisionTest(projectile.position, target.position, projectile.body.m_radius,
                              target.body.m_radius)) {
                // projectile collided with entity

                // destroy projectile
                registry.GetOrEmplace<DeadTag>(projectile.entity);

                // apply damage
                auto damage = projectile.projectileDetails.m_damage;
                if (projectile.entityDetails.m_affinity != target.entityDetails.m_affinity) {
                    damage *= ComplimentAffinityMult;
                }
                SystemDamage::ApplyDamage(registry, target.entity, damage, gamedata);
            }
        }
    }

    void ProcessProjectile(moth::ecs::World& registry, Projectile& projectile, GameData const& gamedata) {
        auto view = registry.View<ComponentBody, ComponentPosition, TargetTag>(moth::ecs::Exclude<DeadTag>);
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

moth::ecs::Entity SystemProjectile::CreateProjectile(moth::ecs::World& registry, ProjectileData const& data,
                                                moth::ecs::Entity source, moth::gfx::FloatVec2 const& position,
                                                moth::gfx::FloatVec2 const& direction, float rotation,
                                                float damage) {
    auto const* sourceEntityData = GetEntityDetails(registry, source);
    if (sourceEntityData == nullptr) {
        return moth::ecs::kNullEntity;
    }
    auto projectile = registry.Create();
    auto& projectileComp = registry.Emplace<ComponentProjectile>(projectile);
    registry.Emplace<ComponentEntity>(projectile, sourceEntityData->m_team, sourceEntityData->m_affinity,
                                      rotation);
    registry.Emplace<ComponentBody>(projectile, data.radius);
    registry.Emplace<ComponentPosition>(projectile, position);
    auto& projectileVel = registry.Emplace<ComponentVelocity>(projectile);

    projectileComp.m_source = source;
    projectileComp.m_damage = damage;

    projectileVel.m_velocity = direction * -data.speed; // -speed because we want -y as up

    auto const& spriteData =
        sourceEntityData->m_affinity == Affinity::Light ? data.white_sprite : data.black_sprite;
    registry.Emplace<ComponentSprite>(projectile, *spriteData);

    if (data.homing_details.has_value()) {
        if (data.homing_details->targeting != Targeting::None) {
            auto& targeting = registry.Emplace<ComponentTargeting>(projectile);
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
    auto view = registry.View<ComponentEntity, ComponentProjectile, ComponentBody, ComponentPosition>(
        moth::ecs::Exclude<DeadTag>);
    for (auto [entity, entityDetails, projectileDetails, body, position] : view.each()) {
        Projectile projectile{ entity, entityDetails, body, projectileDetails, position };
        ProcessProjectile(registry, projectile, gamedata);
    }
}
