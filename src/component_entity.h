#pragma once

#include <canyon/utils/vector.h>
#include "gamedata_enemy.h"

enum class Team {
    NONE = 0,
    PLAYER = 1,
    ENEMY = 2,
};

struct ComponentEntity {
    Team m_team = Team::NONE;
    EnergyColor m_color = EnergyColor::Blue;
    float m_radius = 0;
    float m_angle = 0;

    // cached values from the movement system
    canyon::FloatVec2 m_heading; // the direction we are facing
};

