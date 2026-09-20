#pragma once

#include "gamedata.h"
#include "gamedata_enemy.h"
#include "system_behaviour.h"
#include <moth/ecs/world.h>

class SystemEnemySpawner {
public:
    static moth::ecs::Entity SpawnEnemy(moth::ecs::World& registry, EnemyData const& data,
                                   BehaviourData const& behaviour, moth::gfx::FloatVec2 const& position,
                                   GameData const& gamedata);
};
