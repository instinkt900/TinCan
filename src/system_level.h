#pragma once

#include "gamedata_encounter.h"
#include <entt/entity/fwd.hpp>
#include <vector>

class GameWorld;

struct EncounterState {
    EncounterData const& m_data;
    uint32_t m_currentTime = 0;
    uint32_t m_lastTime = 0;
    uint32_t m_nextEventIdx = 0;
    float m_lastEventSeconds = 0;

    EncounterState(EncounterData const& data)
        : m_data(data) {}
};

struct ComponentLevel {
    std::vector<EncounterData> m_encounters;
    uint32_t m_prerollTime = 4000;
    uint32_t m_encounterIdx = 0;
    uint32_t m_currentTime = 0;

    std::unique_ptr<EncounterState> m_encounterState;
};

class SystemLevel {
public:
    static void InitLevel(entt::registry& registry, uint32_t encounterCount, GameData const& gamedata);
    static void Update(GameWorld& world, uint32_t ticks);
};
