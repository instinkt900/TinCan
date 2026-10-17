#include "system_damage.h"
#include "component_death_sprite.h"
#include "component_body.h"
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

    float const LargeDeathShakeIntensity = 0.8f;
    uint32_t const LargeDeathShakeDuration = 400;
    uint32_t const LargeDeathHitStop = 50;
    float const LargeExplosionScale = 2.0f;
}

void SystemDamage::DestroyPlayer(moth::ecs::World& registry, moth::ecs::Entity player, GameData const& gamedata) {
    if (registry.Has<DeadTag>(player) || SystemRespawn::IsProtected(registry, player)) {
        return;
    }
    registry.Emplace<DeadTag>(player);
    SystemRespawn::OnPlayerDestroyed(registry, player, gamedata);
    SystemPickup::DropWeapons(registry, player, gamedata);

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
            registry.Emplace<DeadTag>(target);
            if (isEnemy) {
                SystemSound::Play(registry, "enemy_death");
            }

            // only a large enemy shakes the screen and stops the game for a moment as it goes
            bool const large = registry.Has<LargeTag>(target);
            if (large) {
                SystemDrawable::StartCameraShake(registry, LargeDeathShakeIntensity, LargeDeathShakeDuration);
                SystemHitStop::Start(registry, LargeDeathHitStop);
            }

            if (auto const* deathSprite = registry.TryGet<ComponentDeathSprite>(target);
                deathSprite != nullptr && deathSprite->m_sprite != nullptr && targetPosition != nullptr) {
                auto const explosion = SystemDrawable::CreateOneShotSprite(registry, *deathSprite->m_sprite,
                                                                           targetPosition->m_position);
                // the explosion is as wide as the enemy, and a large enemy's is bigger again
                auto const* body = registry.TryGet<ComponentBody>(target);
                if (explosion != moth::ecs::kNullEntity && body != nullptr) {
                    auto& sprite = registry.Get<ComponentSprite>(explosion);
                    auto const frameWidth = static_cast<float>(sprite.m_animator->GetSprite().GetWidth());
                    if (frameWidth > 0.0f) {
                        auto const scale =
                            (body->m_radius * 2.0f / frameWidth) * (large ? LargeExplosionScale : 1.0f);
                        sprite.m_scale = sprite.m_scale * scale;
                    }
                }
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
