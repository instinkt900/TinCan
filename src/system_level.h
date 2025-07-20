#pragma once

#include "gamedata_level.h"
#include <entt/entity/fwd.hpp>
#include <vector>

class GameWorld;

struct ComponentLevel {
    std::vector<LevelEvent> m_events;
    uint32_t m_currentTime = 0;
    int32_t m_lastEvent = -1;
    float m_lastEventSeconds = 0;
};

class SystemLevel {
    public:
        static void InitLevel(entt::registry& registry, std::string const& levelName, GameData const& gamedata);
        static void Update(GameWorld& world, uint32_t ticks);
};
