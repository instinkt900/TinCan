#pragma once

#include "gamedata.h"
#include "gamedata_enemy.h"
#include "behaviour_transform.h"
#include "gamedata_pickup.h"
#include "system_behaviour.h"
#include <moth/ecs/world.h>
#include <string>
#include <vector>

// What a spawn of one or more enemies asks for, whatever triggered it: a map's EnemySpawn or an enemy's
// death spawn. Where the formation goes is up to whoever spawns it.
struct EnemySpawnDescription {
    EnemyData const* enemy = nullptr;
    // null spawns the members with no behaviour
    BehaviourData const* behaviour = nullptr;
    BehaviourOverride behaviourOverride;
    // degrees added to the behaviour rotation for each place in the formation, so that a group fans out
    float rotationStep = 0.0f;
    int count = 1;
    std::string formation = "line";
    float spacing = 32.0f;
    moth::gfx::FloatVec2 scale{ 1.0f, 1.0f };
    DataRef<PickupData> drop;
};

// the death spawns of the enemy data an entity was spawned from, which the lifetime system spawns when the
// entity is killed
struct ComponentDeathSpawns {
    std::vector<EnemyDeathSpawn> const* m_spawns = nullptr;
};

class SystemEnemySpawner {
public:
    static moth::ecs::Entity SpawnEnemy(moth::ecs::World& registry, EnemyData const& data,
                                   BehaviourData const& behaviour, moth::gfx::FloatVec2 const& position,
                                   GameData const& gamedata);

    // Spawns the members of a formation around origin, with the formation turned by rotation (radians).
    // Members with no behaviour move at idleVelocity. The members share a group when there is more than
    // one of them or a drop, and also join each of extraGroups.
    static void SpawnFormation(moth::ecs::World& registry, GameData const& gamedata,
                               EnemySpawnDescription const& spawn, moth::gfx::FloatVec2 const& origin,
                               float rotation, moth::gfx::FloatVec2 const& idleVelocity,
                               std::vector<moth::ecs::Entity> const& extraGroups);
};
