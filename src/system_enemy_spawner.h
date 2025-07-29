#pragma once

#include "gamedata.h"
#include "gamedata_spawner.h"
#include "system_behaviour.h"
#include <entt/entity/entity.hpp>

struct ComponentEnemySpawner {
    bool m_active = false;
    SpawnerType m_type = SpawnerType::Unknown;
    canyon::FloatVec2 m_offsetStep;
    int32_t m_count = 0;
    int32_t m_cooldown = 0;
    int32_t m_groupCount = 0;
    int32_t m_groupCooldown = 0;
    int32_t m_maxCooldown = 1000;
    int32_t m_maxGroupCount = 1;
    int32_t m_maxGroupCooldown = 0;
    std::optional<DataRef<PickupData>> m_groupDrop;
    entt::entity m_currentGroupEntity = entt::null;
    EnemyData m_enemy;
    EnemyBehaviour m_behaviour;
    BehaviourParameterList m_behaviourParameters;
    EnemyKillType m_killType;
    std::optional<uint32_t> m_lifetime;
    std::optional<float> m_boundsBorder;

    ComponentEnemySpawner(SpawnerData const& data);
};

class SystemEnemySpawner {
public:
    static void Update(GameWorld& world, uint32_t ticks);

    static entt::entity CreateSpawner(entt::registry& registry, SpawnerData const& data,
                                      GameData const& gamedata, canyon::FloatVec2 const& position);
};
