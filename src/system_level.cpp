#include "system_level.h"
#include <entt/entt.hpp>
#include "game_layer.h"
#include "gamedata.h"
#include "system_enemy_spawner.h"

void SystemLevel::InitLevel(entt::registry& registry, std::string const& levelName,
                            Gamedata const& gamedata) {
    //

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
    levelComp.m_currentEvent = -1;
    levelComp.m_currentTime = 0;

    std::sort(std::begin(levelComp.m_events), std::end(levelComp.m_events),
              [](auto const& eventA, auto const& eventB) { return eventA.time < eventB.time; });
}

void HandleLevelEventSpawn(LevelEvent const& event, entt::registry& registry, Gamedata const& gamedata) {
    BehaviourParameterList params;
    params["speed"] = 200.0f;
    auto newEnemy = SystemEnemySpawner::CreateEnemy(registry, event.name, gamedata, event.location, "straight", params);
    if (event.drop_type.has_value() && event.drop_name.has_value()) {
        auto& drop = registry.emplace<ComponentDrop>(newEnemy);
        drop.m_type = event.drop_type.value();
        drop.m_name = event.drop_name.value();
    }
}

void HandleLevelEventCreateSpawner(LevelEvent const& event, entt::registry& registry, Gamedata const& gamedata) {
    SystemEnemySpawner::CreateSpawner(registry, event.name, gamedata, event.location);
}

void HandleLevelEvent(LevelEvent const& event, entt::registry& registry, Gamedata const& gamedata) {
    switch (event.type) {
    case LevelEventType::Spawn:
        HandleLevelEventSpawn(event, registry, gamedata);
        break;
    case LevelEventType::CreateSpawner:
        HandleLevelEventCreateSpawner(event, registry, gamedata);
        break;
    default:
        break;
    }
}

void SystemLevel::Update(entt::registry& registry, uint32_t ticks, Gamedata const& gamedata) {
    //
    auto view = registry.view<ComponentLevel>();
    if (view.size() > 1) {
        spdlog::warn("More than one level component found.");
    }

    for (auto [entity, level] : view.each()) {
        level.m_currentTime += ticks;
        float const seconds = static_cast<float>(level.m_currentTime) / 1000.0f;

        for (size_t i = (level.m_currentEvent + 1); i < level.m_events.size(); ++i) {
            if (level.m_events[i].time > seconds) {
                break;
            }

            HandleLevelEvent(level.m_events[i], registry, gamedata);
            level.m_currentEvent = i;
        }
    }
}
