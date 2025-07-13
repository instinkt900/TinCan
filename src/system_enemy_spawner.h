#pragma once

#include "gamedata.h"
#include "system_behaviour.h"
#include <entt/entity/entity.hpp>

struct ComponentEnemySpawner {
    bool m_active = false;
    SpawnerType m_type = SpawnerType::Unknown;
    float m_distance = 0;
    int32_t m_count = 0;
    int32_t m_cooldown = 0;
    int32_t m_groupCount = 0;
    int32_t m_groupCooldown = 0;
    int32_t m_maxCooldown = 1000;
    int32_t m_maxGroupCount = 1;
    int32_t m_maxGroupCooldown = 0;
    std::optional<std::string> m_groupDrop;
    entt::entity m_currentGroupEntity = entt::null;
    std::string m_enemyName;
    std::string m_behaviourName;
    BehaviourParameterList m_behaviourParameters;
};

class SystemEnemySpawner {
public:
    static void Update(entt::registry& registry, uint32_t ticks, Gamedata const& gamedata);

    static entt::entity CreateEnemy(entt::registry& registry, std::string const& name,
                                    Gamedata const& gamedata, canyon::FloatVec2 const& position,
                                    std::string const& behaviourName,
                                    BehaviourParameterList const& behaviourParameters);
    static entt::entity CreateSpawner(entt::registry& registry, std::string const& name,
                                      Gamedata const& gamedata, canyon::FloatVec2 const& position);
    static entt::entity CreateSpawner(entt::registry& registry, SpawnerData const& data,
                                      Gamedata const& gamedata, canyon::FloatVec2 const& position);
};
