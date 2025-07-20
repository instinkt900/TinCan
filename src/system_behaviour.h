#pragma once

#include "gamedata_spawner.h"
#include <canyon/utils/vector.h>

class GameWorld;

struct CurveSample {
    canyon::FloatVec2 position;
    float segment_length = 0;
    float total_distance = 0;
};

struct ComponentCurveCache {
    std::vector<CurveSample> samples;
    float total_length;
};


struct ComponentBehaviour {
    EnemyBehaviour m_behaviour;
    uint32_t m_ticks = 0;
    canyon::FloatVec2 m_offset;
    BehaviourParameterList m_parameters;
};

class SystemBehaviour {
public:
    static void Update(GameWorld& world, uint32_t ticks);
};
