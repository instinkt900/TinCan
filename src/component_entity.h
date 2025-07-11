#pragma once

#include <canyon/graphics/color.h>

enum class Team {
    NONE = 0,
    PLAYER = 1,
    ENEMY = 2,
};

enum class EnergyColor {
    WHITE,
    BLACK,
};

inline canyon::graphics::Color GetEnergyColor(EnergyColor const& color) {
    return color == EnergyColor::WHITE ? canyon::graphics::BasicColors::Blue
                                       : canyon::graphics::BasicColors::Red;
}

struct ComponentEntity {
    Team m_team;
    EnergyColor m_color;
    float m_radius;
    float m_angle;
};

