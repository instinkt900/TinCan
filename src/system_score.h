#pragma once

#include <moth/ecs/world.h>
#include <cstdint>

class GameWorld;
class GameData;

// The points an enemy gives when it is killed, from its data.
struct ComponentScoreValue {
    int32_t m_score = 0;
};

class SystemScore {
public:
    // adds points to the run's score, and gives a life for every so many points
    static void Add(moth::ecs::World& registry, int32_t points, GameData const& gamedata);
    static int32_t GetScore(moth::ecs::World const& registry);
};
