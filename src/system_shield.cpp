#include "system_shield.h"
#include "utils_collision.h"
#include "component_body.h"
#include "component_entity.h"
#include "system_drawable.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "gamedata_player.h"
#include "system_projectile.h"
#include "system_hud.h"
#include "system_sound.h"
#include "system_targeting.h"
#include "system_world_bounds.h"
#include <algorithm>
#include <utility>
#include <vector>
#include <moth/ecs/world.h>
#include "game_world.h"
#include "tags.h"

namespace {
    uint32_t const AbsorbDuration = 80;
    uint32_t const PulseDuration = 150;
    float const PulseGrowth = 0.2f;

    void UpdateAbsorbing(moth::ecs::World& registry, uint32_t ticks) {
        std::vector<moth::ecs::Entity> finished;
        auto view = registry.View<ComponentAbsorbing, ComponentPosition>(moth::ecs::Exclude<DeadTag>);
        for (auto [entity, absorbing, position] : view.each()) {
            absorbing.m_elapsed = std::min(AbsorbDuration, absorbing.m_elapsed + ticks);
            auto target = absorbing.m_from;
            if (auto const* shieldPosition = registry.TryGet<ComponentPosition>(absorbing.m_shield)) {
                target = shieldPosition->m_position;
            }
            auto const factor = static_cast<float>(absorbing.m_elapsed) / static_cast<float>(AbsorbDuration);
            position.m_lastPosition = position.m_position;
            position.m_position = moth::gfx::Interp(absorbing.m_from, target, factor, moth::gfx::InterpType::Linear);
            if (absorbing.m_elapsed >= AbsorbDuration) {
                finished.push_back(entity);
            }
        }
        for (auto const entity : finished) {
            registry.Emplace<DeadTag>(entity);
        }
    }

    void UpdatePulse(ComponentShield& shield, ComponentSprite& sprite, uint32_t ticks) {
        if (shield.m_baseScale.x == 0.0f && shield.m_baseScale.y == 0.0f) {
            shield.m_baseScale = sprite.m_scale;
        }
        shield.m_pulseLeft -= std::min(shield.m_pulseLeft, ticks);
        auto const factor = static_cast<float>(shield.m_pulseLeft) / static_cast<float>(PulseDuration);
        sprite.m_scale = shield.m_baseScale * (1.0f + (PulseGrowth * factor));
    }
}

void SystemShield::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    UpdateAbsorbing(registry, ticks);
    // each absorbed bullet and the shield it went into. the bullets are changed after the views are done
    std::vector<std::pair<moth::ecs::Entity, moth::ecs::Entity>> absorbed;
    // each absorbed bullet is worth the same, whatever its damage
    float absorbPower = 0.0f;
    if (auto const* playerData = world.GetGameData().GetPlayer(); playerData != nullptr && playerData->power_weapon) {
        absorbPower = playerData->power_weapon->absorb_power;
    }
    auto view = registry.View<ComponentParenting, ComponentPosition, ComponentBody, ComponentShield>(
        moth::ecs::Exclude<DeadTag>);

    for (auto [shieldEntity, shieldParenting, shieldPosition, shieldBody, shield] : view.each()) {
        if (!registry.Valid(shieldParenting.m_parent)) {
            continue;
        }

        auto const* shieldDetails = registry.TryGet<ComponentEntity>(shieldParenting.m_parent);
        if (shieldDetails == nullptr) {
            continue;
        }

        // update the visuals
        if (shield.m_shownAffinity != shieldDetails->m_affinity) {
            shield.m_shownAffinity = shieldDetails->m_affinity;
            if (auto* shieldSprite = registry.TryGet<ComponentSprite>(shieldEntity)) {
                SetSpriteState(*shieldSprite,
                               shieldDetails->m_affinity == Affinity::Dark ? "idle_dark" : "idle_light");
            }
        }

        auto projectileView =
            registry.View<ComponentProjectile, ComponentEntity, ComponentBody, ComponentPosition>();

        for (auto [projectileEntity, projectileData, projectileDetails, projectileBody, projectilePosition] :
             projectileView.each()) {
            if (projectileDetails.m_affinity == shieldDetails->m_affinity &&
                projectileDetails.m_team != shieldDetails->m_team) {

                auto const t = SweepTest(shieldPosition.m_position, shieldPosition.m_lastPosition,
                                         shieldBody.m_radius, projectilePosition.m_position,
                                         projectilePosition.m_lastPosition, projectileBody.m_radius);
                if (t < 1.0f) {
                    // projectile collided with shield
                    absorbed.emplace_back(projectileEntity, shieldEntity);
                    SystemSound::Play(registry, "absorb");
                    SystemHud::TickPower(registry);
                    shield.m_pulseLeft = PulseDuration;

                    // apply power
                    if (auto* ownerPower = registry.TryGet<ComponentPower>(shieldParenting.m_parent)) {
                        ownerPower->Add(absorbPower);
                    }
                }
            }
        }

        if (auto* shieldSprite = registry.TryGet<ComponentSprite>(shieldEntity)) {
            UpdatePulse(shield, *shieldSprite, ticks);
        }
    }

    // an absorbed bullet stops being a projectile and loses its body, so nothing else can hit with it, and pulls
    // into the shield from where it was caught
    for (auto const& [projectile, shieldEntity] : absorbed) {
        if (registry.Has<ComponentAbsorbing>(projectile)) {
            continue;
        }
        registry.Remove<ComponentProjectile>(projectile);
        registry.Remove<ComponentBody>(projectile);
        registry.Remove<ComponentVelocity>(projectile);
        registry.Remove<ComponentBounds>(projectile);
        registry.Remove<ComponentTargeting>(projectile);
        auto& absorbing = registry.Emplace<ComponentAbsorbing>(projectile);
        absorbing.m_shield = shieldEntity;
        absorbing.m_from = registry.Get<ComponentPosition>(projectile).m_position;
    }
}
