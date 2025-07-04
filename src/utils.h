#pragma once

#include "system_projectile.h"
#include <canyon/graphics/color.h>

static canyon::graphics::Color GetEnergyColor(EnergyColor const& color) {
    return color == EnergyColor::WHITE ? canyon::graphics::BasicColors::Blue
                                       : canyon::graphics::BasicColors::Red;
}

