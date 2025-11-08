#include "system_level.h"
#include <entt/entt.hpp>
#include "gamedata.h"
#include "system_drawable.h"
#include "system_enemy_spawner.h"
#include "game_world.h"
#include "system_pickup.h"
#include "utils.h"

void BuildLevelEncounters(ComponentLevel& levelComponent, LevelBuilderProperties const& properties, GameData const& gamedata) {
    auto const& encounterDatabase = gamedata.Get<EncounterData>();
    for (uint32_t i = 0; i < properties.encounterCount; ++i) {
        auto keys = encounterDatabase->Keys();
        auto randIdx = Random<size_t>(0, encounterDatabase->Size() - 1);
        spdlog::info("Encounter {}", keys[randIdx]);
        auto const* encounterData = encounterDatabase->Get(keys[randIdx]);
        levelComponent.m_encounters.emplace_back(*encounterData);
    }
}

void SystemLevel::InitLevel(entt::registry& registry, LevelBuilderProperties const& levelProperties, GameData const& gamedata) {
    auto view = registry.view<ComponentLevel>();
    if (!view.empty()) {
        spdlog::error("Level already exists.");
        return;
    }

    auto levelEntity = registry.create();
    auto& levelComp = registry.emplace<ComponentLevel>(levelEntity);

    BuildLevelEncounters(levelComp, levelProperties, gamedata);

    levelComp.m_encounterIdx = 0;
    levelComp.m_currentTime = 0;

    registry.emplace<ComponentCamera>(levelEntity);
}

void HandleLevelEvent(LevelEvent const& event, GameWorld& world) {
    auto& registry = world.GetRegistry();
    auto const& gamedata = world.GetGameData();
    auto const worldLocation = event.location * static_cast<canyon::FloatVec2>(world.GetWorldSize());
    if (event.spawner.valid()) {
        SystemEnemySpawner::CreateSpawner(registry, *event.spawner, world.GetGameData(), worldLocation);
    } else if (event.drop.valid()) {
        SystemPickup::CreatePickup(registry, worldLocation, *event.drop, gamedata);
    }
}

// returns false when the encounter is over
bool UpdateEncounter(EncounterState& encounter, uint32_t ticks, GameWorld& world) {
    encounter.m_currentTime += ticks;
    float const seconds = static_cast<float>(encounter.m_currentTime) / 1000.0f;

    auto const eventCount = encounter.m_data.events.size();
    while ((encounter.m_nextEventIdx < eventCount) &&
           (encounter.m_data.events[encounter.m_nextEventIdx].offset <=
            (seconds - encounter.m_lastEventSeconds))) {
        HandleLevelEvent(encounter.m_data.events[encounter.m_nextEventIdx], world);
        encounter.m_nextEventIdx++;
        encounter.m_lastEventSeconds = seconds;
    }

    auto const maxTime = static_cast<uint32_t>(encounter.m_data.duration * 1000);
    return encounter.m_currentTime < maxTime;
}

void SystemLevel::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto view = registry.view<ComponentLevel>();
    if (view.size() > 1) {
        spdlog::warn("More than one level component found.");
    }

    for (auto [entity, level] : view.each()) {
        level.m_currentTime += ticks;

        auto const encounterCount = level.m_encounters.size();
        if (level.m_prerollTime > 0) {
            level.m_prerollTime -= ticks;
        } else if (level.m_encounterIdx < encounterCount) {
            if (!level.m_encounterState) {
                if (level.m_encounterIdx < encounterCount) {
                    auto const& nextEncounter = level.m_encounters[level.m_encounterIdx];
                    level.m_encounterState = std::make_unique<EncounterState>(nextEncounter);
                }
            }
            if (!UpdateEncounter(*level.m_encounterState, ticks, world)) {
                level.m_encounterState.reset();
                level.m_encounterIdx++;
            }
        }
    }
}
