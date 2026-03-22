#pragma once

#include "game_world.h"
#include "utils_paths.h"

struct ComponentDebugSpline {
    std::string m_label;
    bool m_visible = true;
    SplineDefinition m_spline;
};

namespace imgui_ext {
    void Details(ComponentDebugSpline& spline);
};

void DrawSpline(moth_graphics::graphics::IGraphics& graphics, std::vector<moth_graphics::FloatVec2> const& points,
                moth_graphics::FloatVec2 const& offset = { 0, 0 });

class SystemDebugSpline {
public:
    static void Draw(GameWorld& world, moth_graphics::graphics::IGraphics& graphics);
};
