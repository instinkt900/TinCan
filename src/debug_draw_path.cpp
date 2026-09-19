#include "debug_draw_path.h"

void DrawPath(moth::gfx::IGraphics& graphics, std::vector<moth::gfx::FloatVec2> const& points,
              PathInterpolation interpolation, moth::gfx::FloatVec2 const& offset) {
    auto const samples = 100;
    auto const tStep = 1.0f / static_cast<float>(samples);
    auto t = 0.0f;
    auto lastSample = SamplePath(points, interpolation, t);
    graphics.SetColor(moth::gfx::FromARGB(0xFFFFFF00));
    for (auto i = 1; i <= samples; ++i) {
        t = tStep * static_cast<float>(i);
        auto const newSample = SamplePath(points, interpolation, t) + offset;
        graphics.DrawLineF(lastSample, newSample);
        lastSample = newSample;
    }
    graphics.SetColor(moth::gfx::FromARGB(0xFFFF0000));
    for (uint i = 0; i < points.size(); ++i) {
        auto const pos = points[i] + offset;
        auto const v1 = pos - moth::gfx::FloatVec2{ 0.0f, 6.0f };
        auto const v2 = pos + moth::gfx::FloatVec2{ 0.0f, 6.0f };
        auto const h1 = pos - moth::gfx::FloatVec2{ 6.0f, 0.0f };
        auto const h2 = pos + moth::gfx::FloatVec2{ 6.0f, 0.0f };
        graphics.DrawLineF(v1, v2);
        graphics.DrawLineF(h1, h2);
    }
}
