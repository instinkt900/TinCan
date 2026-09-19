#include "debug_draw_path.h"

namespace {
    void DrawCross(moth::gfx::IGraphics& graphics, moth::gfx::FloatVec2 const& pos, float size) {
        graphics.DrawLineF(pos - moth::gfx::FloatVec2{ 0.0f, size }, pos + moth::gfx::FloatVec2{ 0.0f, size });
        graphics.DrawLineF(pos - moth::gfx::FloatVec2{ size, 0.0f }, pos + moth::gfx::FloatVec2{ size, 0.0f });
    }
}

void DrawPath(moth::gfx::IGraphics& graphics, std::vector<moth::gfx::FloatVec2> const& points,
              PathInterpolation interpolation, moth::gfx::FloatVec2 const& offset) {
    auto const samples = 100;
    auto const tStep = 1.0f / static_cast<float>(samples);
    auto t = 0.0f;
    auto lastSample = SamplePath(points, interpolation, t) + offset;
    graphics.SetColor(moth::gfx::FromARGB(0xFFFFFF00));
    for (auto i = 1; i <= samples; ++i) {
        t = tStep * static_cast<float>(i);
        auto const newSample = SamplePath(points, interpolation, t) + offset;
        graphics.DrawLineF(lastSample, newSample);
        lastSample = newSample;
    }
}

void DrawWaypointHandles(moth::gfx::IGraphics& graphics, std::vector<moth::gfx::FloatVec2> const& points,
                         moth::gfx::FloatVec2 const& offset, int selectedIndex) {
    for (int i = 0; i < static_cast<int>(points.size()); ++i) {
        graphics.SetColor(i == selectedIndex ? moth::gfx::FromARGB(0xFF00FF00)
                                             : moth::gfx::FromARGB(0xFFFF0000));
        DrawCross(graphics, points[i] + offset, WaypointHandleRadius / 2.0f);
    }
}
