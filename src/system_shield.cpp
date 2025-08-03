#include "system_shield.h"
#include "utils_collision.h"
#include "component_body.h"
#include "component_entity.h"
#include "system_drawable.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_projectile.h"
#include <entt/entt.hpp>
#include "game_world.h"
#include "tags.h"

void SystemShield::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto view = registry.view<ComponentParenting, ComponentPosition, ComponentBody, ComponentShield>(
        entt::exclude<DeadTag>);

    for (auto [shieldEntity, shieldParenting, shieldPosition, shieldBody] : view.each()) {
        if (!registry.valid(shieldParenting.m_parent)) {
            continue;
        }

        auto const* shieldDetails = registry.try_get<ComponentEntity>(shieldParenting.m_parent);
        if (shieldDetails == nullptr) {
            continue;
        }

        // update the visuals
        if (auto* shieldSprite = registry.try_get<ComponentSprite>(shieldEntity)) {
            shieldSprite->m_color = (shieldDetails->m_affinity == Affinity::Light)
                                        ? canyon::graphics::BasicColors::Blue
                                        : canyon::graphics::BasicColors::Red;

            float const imageWidth = static_cast<float>(shieldSprite->m_image->GetWidth());
            float const imageHeight = static_cast<float>(shieldSprite->m_image->GetHeight());
            float const widthScale = (shieldBody.m_radius * 2.0f) / imageWidth;
            float const heightScale = (shieldBody.m_radius * 2.0f) / imageHeight;
            shieldSprite->m_scale.x = widthScale;
            shieldSprite->m_scale.y = heightScale;
        }

        auto projectileView =
            registry.view<ComponentProjectile, ComponentEntity, ComponentBody, ComponentPosition>();

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
                    registry.get_or_emplace<DeadTag>(projectileEntity);

                    // apply power
                    if (auto* ownerPower = registry.try_get<ComponentPower>(shieldParenting.m_parent)) {
                        ownerPower->m_power += projectileData.m_damage;
                    }
                }
            }
        }
    }
}
