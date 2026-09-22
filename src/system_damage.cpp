#include "system_damage.h"
#include "component_death_sprite.h"
#include "component_drop.h"
#include "component_health.h"
#include "gamedata.h"
#include "gamedata_pickup.h"
#include "system_drawable.h"
#include "system_movement.h"
#include "system_pickup.h"
#include "tags.h"
#include <spdlog/spdlog.h>

void SystemDamage::ApplyDamage(moth::ecs::World& registry, moth::ecs::Entity target, float damage,
                               GameData const& gamedata) {
    auto* entityHealth = registry.TryGet<ComponentHealth>(target);
    if (entityHealth == nullptr) {
        return;
    }

    auto const* targetPosition = registry.TryGet<ComponentPosition>(target);

    if (entityHealth->m_currentHealth > 0) {
        entityHealth->m_currentHealth -= damage;
        if (entityHealth->m_currentHealth <= 0) {
            SystemDrawable::StartCameraShake(registry, 0.5f, 500);
            registry.Emplace<DeadTag>(target);

            if (auto const* deathSprite = registry.TryGet<ComponentDeathSprite>(target);
                deathSprite != nullptr && deathSprite->m_sprite != nullptr && targetPosition != nullptr) {
                SystemDrawable::CreateOneShotSprite(registry, *deathSprite->m_sprite,
                                                    targetPosition->m_position);
            }

            if (auto const* dropComponent = registry.TryGet<ComponentDrop>(target)) {
                auto const* pickupData = gamedata.Get<PickupData>(dropComponent->m_name);
                if (pickupData == nullptr) {
                    spdlog::error("Unknown pickup {}", dropComponent->m_name);
                    return;
                }
                if (targetPosition != nullptr) {
                    SystemPickup::CreatePickup(registry, targetPosition->m_position, *pickupData, gamedata);
                }
            }
        }
    }

    // flash damage indicator
    if (auto* sprite = registry.TryGet<ComponentSprite>(target)) {
        sprite->m_flashTime = 0.2f;
    }
}
