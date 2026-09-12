#pragma once

#include "game_world.h"
#include "gamedata_projectile.h"
#include <moth/ecs/world.h>

struct ComponentTargeting {
    Targeting m_targeting = Targeting::Nearest;
    moth::ecs::Entity m_currentTarget = moth::ecs::kNullEntity;
    
    float m_maxTurnSpeed = 0;
    float m_maxThrust = 0;
    float m_maxSpeed = 0;
    float m_minDrag = 0;
    float m_maxDrag = 0;

    float m_turnSpeed = 0;

    // cached values from the movement system
    bool m_targetValid = false;  // if the current target is set and alive
    moth::gfx::FloatVec2 m_bearing; // the direction to our current target
};

class SystemTargeting {
public:
    static void Update(GameWorld& gameworld, uint32_t ticks);
};
