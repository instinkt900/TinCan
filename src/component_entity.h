#pragma once

#include "utils.h"
#include <canyon/graphics/color.h>
#include <canyon/utils/vector.h>

enum class Team {
    NONE = 0,
    PLAYER = 1,
    ENEMY = 2,
};

enum class EnergyColor {
    Blue,
    Red,
};

MAGIC_SERIALIZE_ENUM(EnergyColor);

inline canyon::graphics::Color GetEnergyColor(EnergyColor const& color) {
    return color == EnergyColor::Blue ? canyon::graphics::BasicColors::Blue
                                       : canyon::graphics::BasicColors::Red;
}

struct ComponentEntity {
    Team m_team;
    EnergyColor m_color;
    float m_radius;
    float m_angle;

    // cached values from the movement system
    canyon::FloatVec2 m_heading; // the direction we are facing
};

