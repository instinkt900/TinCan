#pragma once

#include "gamedata.h"
#include "gamedata_spawner.h"
#include "system_behaviour.h"
#include <moth/ecs/world.h>

struct ComponentEnemySpawner {
    bool m_active = false;
    SpawnerType m_type = SpawnerType::Unknown;
    moth::gfx::FloatVec2 m_offsetStep;
    std::vector<moth::gfx::FloatVec2> m_formationPositions;
    int32_t m_count = 0;
    int32_t m_cooldown = 0;
    int32_t m_groupCount = 0;
    int32_t m_groupCooldown = 0;
    int32_t m_maxCooldown = 1000;
    int32_t m_maxGroupCount = 1;
    int32_t m_maxGroupCooldown = 0;
    DataRef<PickupData> m_groupDrop;
    moth::ecs::Entity m_currentGroupEntity = moth::ecs::kNullEntity;
    EnemyData m_enemy;
    BehaviourData m_behaviour;
    EnemyKillType m_killType;
    std::optional<uint32_t> m_lifetime;
    std::optional<float> m_boundsBorder;
    uint32_t m_tickOffset;

    ComponentEnemySpawner(SpawnerData const& data);
};

class SystemEnemySpawner {
public:
    static void Update(GameWorld& world, uint32_t ticks);

    static moth::ecs::Entity CreateSpawner(moth::ecs::World& registry, SpawnerData const& data,
                                      GameData const& gamedata, moth::gfx::FloatVec2 const& position);
    static moth::ecs::Entity SpawnEnemy(moth::ecs::World& registry, EnemyData const& data,
                                   BehaviourData const& behaviour, moth::gfx::FloatVec2 const& position,
                                   GameData const& gamedata);
};
