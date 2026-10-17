#include "system_shield.h"
#include "utils_collision.h"
#include "component_body.h"
#include "component_entity.h"
#include "system_drawable.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "gamedata_player.h"
#include "system_projectile.h"
#include "system_sound.h"
#include <moth/ecs/world.h>
#include "game_world.h"
#include "tags.h"

void SystemShield::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
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

                    // destroy projectile
                    registry.GetOrEmplace<DeadTag>(projectileEntity);
                    SystemSound::Play(registry, "absorb");

                    // apply power
                    if (auto* ownerPower = registry.TryGet<ComponentPower>(shieldParenting.m_parent)) {
                        ownerPower->Add(absorbPower);
                    }
                }
            }
        }
    }
}
