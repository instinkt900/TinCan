#include "system_level.h"
#include <entt/entt.hpp>
#include "gamedata.h"
#include "system_enemy_spawner.h"
#include "game_world.h"
#include "system_pickup.h"

void SystemLevel::InitLevel(entt::registry& registry, std::string const& levelName,
                            GameData const& gamedata) {
    auto view = registry.view<ComponentLevel>();
    if (!view.empty()) {
        spdlog::error("Level already exists.");
        return;
    }

    auto const* levelData = gamedata.GetLevelDatabase().Get(levelName);
    if (levelData == nullptr) {
        spdlog::error("Unable to find level entry {}", levelName);
        return;
    }

    auto levelEntity = registry.create();
    auto& levelComp = registry.emplace<ComponentLevel>(levelEntity);
    levelComp.m_events = levelData->events;
    levelComp.m_lastEvent = -1;
    levelComp.m_currentTime = 0;
    levelComp.m_lastEventSeconds = 0;
}

void HandleLevelEvent(LevelEvent const& event, GameWorld& world) {
    auto& registry = world.GetRegistry();
    auto const& gamedata = world.GetGameData();
    auto worldLocation = event.location * static_cast<canyon::FloatVec2>(world.GetWorldSize());
    if (event.spawner.has_value()) {
        SystemEnemySpawner::CreateSpawner(registry, event.spawner.value(), world.GetGameData(),
                                          worldLocation);
    } else if (event.drop_name.has_value()) {
        SystemPickup::CreatePickup(registry, worldLocation, event.drop_name.value(), gamedata);
    }
}

void SystemLevel::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto view = registry.view<ComponentLevel>();
    if (view.size() > 1) {
        spdlog::warn("More than one level component found.");
    }

    for (auto [entity, level] : view.each()) {
        level.m_currentTime += ticks;
        float const seconds = static_cast<float>(level.m_currentTime) / 1000.0f;

        while (((level.m_lastEvent + 1) < static_cast<int32_t>(level.m_events.size())) &&
               (level.m_events[level.m_lastEvent + 1].offset <= (seconds - level.m_lastEventSeconds))) {
            HandleLevelEvent(level.m_events[level.m_lastEvent + 1], world);
            level.m_lastEvent++;
            level.m_lastEventSeconds = seconds;
        }
    }
}
