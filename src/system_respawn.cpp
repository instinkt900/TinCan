#include "system_respawn.h"
#include "game_world.h"
#include "gamedata_player.h"
#include "system_drawable.h"
#include "system_input.h"
#include "system_movement.h"
#include "system_weapon.h"
#include "system_world_bounds.h"
#include "tags.h"
#include <algorithm>
#include <vector>

namespace {
    // how long the ship stays shown, then hidden, while it blinks
    uint32_t const BlinkInterval = 100;

    ComponentRun* FindRun(moth::ecs::World& registry) {
        auto view = registry.View<ComponentRun>();
        for (auto [entity, run] : view.each()) {
            return &run;
        }
        return nullptr;
    }

    void UpdateFlyIns(GameWorld& world, uint32_t ticks) {
        auto& registry = world.GetRegistry();
        auto const* playerData = world.GetGameData().GetPlayer();
        std::vector<moth::ecs::Entity> arrived;
        auto view = registry.View<ComponentFlyIn, ComponentPosition>(moth::ecs::Exclude<DeadTag>);
        for (auto [entity, flyIn, position] : view.each()) {
            flyIn.m_elapsed = std::min(flyIn.m_duration, flyIn.m_elapsed + ticks);
            auto const factor =
                flyIn.m_duration > 0 ? static_cast<float>(flyIn.m_elapsed) / static_cast<float>(flyIn.m_duration) : 1.0f;
            position.m_lastPosition = position.m_position;
            position.m_position = moth::gfx::Interp(flyIn.m_from, flyIn.m_to, factor, moth::gfx::InterpType::Smooth);
            if (flyIn.m_elapsed >= flyIn.m_duration) {
                arrived.push_back(entity);
            }
        }

        // the ship gets control on arrival, so the components that give it are added once the view is done
        for (auto const entity : arrived) {
            registry.Remove<ComponentFlyIn>(entity);

            auto& input = registry.Emplace<ComponentInput>(entity);
            auto inputView = registry.View<ComponentRun, ComponentInput>();
            for (auto [runEntity, run, runInput] : inputView.each()) {
                input.m_held = runInput.m_held;
            }

            auto& bounds = registry.Emplace<ComponentBounds>(entity);
            bounds.m_behaviour = BoundsBehaviour::Limit;
            auto const& worldSize = world.GetWorldSize();
            bounds.m_bounds = moth::gfx::MakeRect(0.0f, 0.0f, static_cast<float>(worldSize.x),
                                                  static_cast<float>(worldSize.y));

            auto& safe = registry.Emplace<ComponentSafe>(entity);
            safe.m_msLeft = playerData != nullptr ? playerData->safe_time : 0;
        }
    }

    void UpdateSafe(moth::ecs::World& registry, uint32_t ticks) {
        std::vector<moth::ecs::Entity> finished;
        auto view = registry.View<ComponentSafe, ComponentSprite>(moth::ecs::Exclude<DeadTag>);
        for (auto [entity, safe, sprite] : view.each()) {
            safe.m_elapsed += ticks;
            safe.m_msLeft -= std::min(safe.m_msLeft, ticks);
            sprite.m_visible = safe.m_msLeft == 0 || (safe.m_elapsed / BlinkInterval) % 2 == 0;
            if (safe.m_msLeft == 0) {
                finished.push_back(entity);
            }
        }
        for (auto const entity : finished) {
            registry.Remove<ComponentSafe>(entity);
        }
    }
}

void SystemRespawn::OnPlayerDestroyed(moth::ecs::World& registry, moth::ecs::Entity player,
                                      GameData const& gamedata) {
    auto* run = FindRun(registry);
    auto const* playerData = gamedata.GetPlayer();
    if (run == nullptr || playerData == nullptr) {
        return;
    }

    run->m_mainGunLevel = 0;
    if (auto const* weapons = registry.TryGet<ComponentWeapon>(player)) {
        for (auto const& weapon : weapons->m_weapons) {
            if (weapon.m_progression == playerData->weapon) {
                run->m_mainGunLevel = weapon.m_level;
            }
        }
    }

    run->m_lives = std::max(0, run->m_lives - 1);
    if (run->m_lives > 0) {
        run->m_respawning = true;
        run->m_respawnLeft = playerData->respawn_delay;
    }
}

bool SystemRespawn::IsProtected(moth::ecs::World& registry, moth::ecs::Entity player) {
    return registry.Has<ComponentFlyIn>(player) || registry.Has<ComponentSafe>(player);
}

int32_t SystemRespawn::GetLives(moth::ecs::World const& registry) {
    auto view = registry.View<ComponentRun>();
    for (auto [entity, run] : view.each()) {
        return run.m_lives;
    }
    return 0;
}

void SystemRespawn::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    if (auto* run = FindRun(registry); run != nullptr && run->m_respawning) {
        run->m_respawnLeft -= std::min(run->m_respawnLeft, ticks);
        if (run->m_respawnLeft == 0) {
            run->m_respawning = false;
            world.RespawnPlayer(run->m_mainGunLevel);
        }
    }

    UpdateFlyIns(world, ticks);
    UpdateSafe(registry, ticks);
}
