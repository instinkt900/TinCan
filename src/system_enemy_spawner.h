#pragma once

#include "gamedata.h"
#include "system_behaviour.h"
#include <entt/entity/entity.hpp>

struct ComponentEnemySpawner {
    bool m_active = false;
    int32_t m_count = 0;
    int32_t m_cooldown = 0;
    int32_t m_groupCount = 0;
    int32_t m_groupCooldown = 0;
    int32_t m_maxCooldown = 1000;
    int32_t m_maxGroupCount = 1;
    int32_t m_maxGroupCooldown = 0;
    entt::entity m_currentGroupEntity = entt::null;
    std::string m_enemyName;
    std::string m_behaviourName;
    BehaviourParameterList m_behaviourParameters;
};

class SystemEnemySpawner {
public:
    static void Update(entt::registry& registry, uint32_t ticks, Gamedata const& gamedata);

    static entt::entity CreateSpawner(entt::registry& registry, Gamedata const& gamedata,
                                      moth_ui::FloatVec2 const& position);
};
