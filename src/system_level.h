#pragma once

#include "gamedata_level.h"
#include <entt/entity/fwd.hpp>
#include <vector>

struct ComponentLevel {
    std::vector<LevelEvent> m_events;
    uint32_t m_currentTime = 0;
    size_t m_currentEvent = -1;
};

class SystemLevel {
    public:
        static void InitLevel(entt::registry& registry, std::string const& levelName, Gamedata const& gamedata);
        static void Update(entt::registry& registry, uint32_t ticks, Gamedata const& gamedata);
};
