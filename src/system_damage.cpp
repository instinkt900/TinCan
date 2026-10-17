#include "system_damage.h"
#include "component_death_sprite.h"
#include "component_drop.h"
#include "component_health.h"
#include "gamedata.h"
#include "gamedata_pickup.h"
#include "gamedata_player.h"
#include "system_drawable.h"
#include "system_hit_stop.h"
#include "system_movement.h"
#include "system_pickup.h"
#include "system_respawn.h"
#include "system_sound.h"
#include "tags.h"
#include "utils_entity.h"
#include <spdlog/spdlog.h>

namespace {
    // the player's death is the biggest moment in the fight, so its shake is stronger than any other
    float const PlayerDeathShakeIntensity = 1.5f;
    uint32_t const PlayerDeathShakeDuration = 800;
    uint32_t const PlayerDeathHitStop = 100;
}

void SystemDamage::DestroyPlayer(moth::ecs::World& registry, moth::ecs::Entity player, GameData const& gamedata) {
    if (registry.Has<DeadTag>(player) || SystemRespawn::IsProtected(registry, player)) {
        return;
    }
    registry.Emplace<DeadTag>(player);
    SystemRespawn::OnPlayerDestroyed(registry, player, gamedata);

    if (auto const* position = registry.TryGet<ComponentPosition>(player)) {
        auto const* playerData = gamedata.GetPlayer();
        if (auto const* deathSprite = registry.TryGet<ComponentDeathSprite>(player);
            deathSprite != nullptr && deathSprite->m_sprite != nullptr) {
            auto const explosion =
                SystemDrawable::CreateOneShotSprite(registry, *deathSprite->m_sprite, position->m_position);
            if (explosion != moth::ecs::kNullEntity && playerData != nullptr) {
                auto& sprite = registry.Get<ComponentSprite>(explosion);
                sprite.m_scale = { playerData->death_sprite_scale, playerData->death_sprite_scale };
            }
        }
    }

    SystemDrawable::StartCameraShake(registry, PlayerDeathShakeIntensity, PlayerDeathShakeDuration);
    SystemHitStop::Start(registry, PlayerDeathHitStop);
    SystemSound::Play(registry, "player_death");
}

void SystemDamage::ApplyDamage(moth::ecs::World& registry, moth::ecs::Entity target, float damage,
                               GameData const& gamedata) {
    if (registry.Has<PlayerTag>(target)) {
        if (damage > 0.0f) {
            DestroyPlayer(registry, target, gamedata);
        }
        return;
    }

    auto* entityHealth = registry.TryGet<ComponentHealth>(target);
    if (entityHealth == nullptr) {
        return;
    }

    auto const* targetPosition = registry.TryGet<ComponentPosition>(target);
    auto const* targetDetails = GetEntityDetails(registry, target);
    bool const isEnemy = targetDetails != nullptr && targetDetails->m_team == Team::Enemy;

    if (entityHealth->m_currentHealth > 0) {
        entityHealth->m_currentHealth -= damage;
        if (entityHealth->m_currentHealth <= 0) {
            SystemDrawable::StartCameraShake(registry, 0.5f, 500);
            registry.Emplace<DeadTag>(target);
            if (isEnemy) {
                SystemSound::Play(registry, "enemy_death");
            }

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

    if (isEnemy) {
        SystemSound::Play(registry, "hit");
    }

    // flash damage indicator
    if (auto* sprite = registry.TryGet<ComponentSprite>(target)) {
        sprite->m_flashTime = 0.2f;
    }
}
