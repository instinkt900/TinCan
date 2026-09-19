#pragma once

#include "gamedata_behaviour.h"
#include "utils_paths.h"
#include <moth/graphics/utils/vector.h>

class GameWorld;

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
    // where the entity was when the behaviour started. a spawn-relative behaviour measures every one of
    // its states from here, so that the states hold the shape they were authored in, rather than each one
    // starting from wherever the state before it happened to end
    moth::gfx::FloatVec2 m_origin{ 0, 0 };
    bool m_init = false;
    // place in the formation this entity was spawned with, which is what a Phase modifier multiplies its
    // offset by so that members of one formation do not move in lockstep
    uint32_t m_phaseIndex = 0;
};

// Where a path state puts an entity that has travelled this far along it, with the state's modifiers
// applied. The engine fixes the order they apply in: Phase feeds the wave, Ease reshapes the progress
// along the path, then Wave displaces across the direction of travel.
moth::gfx::FloatVec2 SamplePathState(BehaviourStateData const& state, ArcLengthLUT const& lut,
                                     float distanceTravelled, uint32_t phaseIndex);

class SystemBehaviour {
public:
    static void Update(GameWorld& world, uint32_t ticks);
};
