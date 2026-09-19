#pragma once

#include "gamedata_behaviour.h"
#include "utils_paths.h"
#include <moth/graphics/utils/vector.h>

class GameWorld;

struct CurveSample {
    moth::gfx::FloatVec2 position;
    float segment_length = 0;
    float total_distance = 0;
};

struct ComponentCurveCache {
    std::vector<CurveSample> samples;
    float total_length;
};

// what a path state works out once when it is entered: where its coordinates are measured from, and the
// arc-length table that turns a distance travelled into a point on the path
struct ComponentPathCache {
    moth::gfx::FloatVec2 m_offset;
    ArcLengthLUT m_lut;
};

struct ComponentBehaviour {
    BehaviourData m_data;
    BehaviourStateData const* m_currentState = nullptr;
    uint32_t m_stateIdx = 0;
    uint32_t m_stateTime = 0;
    bool m_init = false;
};

class SystemBehaviour {
public:
    static void Update(GameWorld& world, uint32_t ticks);
};
