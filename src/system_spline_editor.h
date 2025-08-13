#pragma once

#include "game_world.h"
#include "utils_paths.h"

struct ComponentSpline {
    std::string m_name;
    SplineDefinition m_spline;
};

class SystemSplineEditor {
public:
    static void Draw(GameWorld& world, canyon::graphics::IGraphics& graphics);
};
