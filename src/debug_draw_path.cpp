#include "debug_draw_path.h"

namespace {
    void DrawCross(moth::gfx::IGraphics& graphics, moth::gfx::FloatVec2 const& pos, float size) {
        graphics.DrawLineF(pos - moth::gfx::FloatVec2{ 0.0f, size }, pos + moth::gfx::FloatVec2{ 0.0f, size });
        graphics.DrawLineF(pos - moth::gfx::FloatVec2{ size, 0.0f }, pos + moth::gfx::FloatVec2{ size, 0.0f });
    }

    // the flown position at this fraction of the path, modifiers and all
    moth::gfx::FloatVec2 FlownAt(BehaviourStateData const& state, ArcLengthLUT const& lut, float progress) {
        return SamplePathState(state, lut, progress * lut.distances.back(), 0);
    }

    // the direction of travel at this fraction of the path, taken from the flown path so that a wave shows
    // as a wave rather than as the line underneath it
    moth::gfx::FloatVec2 FlownDirection(BehaviourStateData const& state, ArcLengthLUT const& lut,
                                        float progress) {
        float const step = 0.002f;
        auto const delta = FlownAt(state, lut, std::min(1.0f, progress + step)) -
                           FlownAt(state, lut, std::max(0.0f, progress - step));
        if (moth::gfx::LengthSq(delta) <= 0.0f) {
            return { 0, 1 };
        }
        return moth::gfx::Normalized(delta);
    }

    void DrawArrow(moth::gfx::IGraphics& graphics, moth::gfx::FloatVec2 const& pos,
                   moth::gfx::FloatVec2 const& direction, float size) {
        auto const back = pos - (direction * size);
        auto const side = moth::gfx::FloatVec2{ -direction.y, direction.x } * (size * 0.5f);
        graphics.DrawLineF(pos, back + side);
        graphics.DrawLineF(pos, back - side);
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

void DrawPathStatePreview(moth::gfx::IGraphics& graphics, BehaviourStateData const& state,
                          ArcLengthLUT const& lut, moth::gfx::FloatVec2 const& offset, float progress) {
    auto const totalLength = lut.distances.back();
    if (totalLength <= 0.0f) {
        return;
    }

    // the flown path, drawn over the plain geometry so the two can be compared
    auto const samples = 200;
    auto const step = 1.0f / static_cast<float>(samples);
    graphics.SetColor(moth::gfx::FromARGB(0xFF66CCFF));
    auto last = FlownAt(state, lut, 0.0f) + offset;
    for (auto i = 1; i <= samples; ++i) {
        auto const next = FlownAt(state, lut, step * static_cast<float>(i)) + offset;
        graphics.DrawLineF(last, next);
        last = next;
    }

    // which way along it the entity goes
    for (auto i = 1; i <= 8; ++i) {
        auto const u = static_cast<float>(i) / 9.0f;
        auto const pos = FlownAt(state, lut, u) + offset;
        DrawArrow(graphics, pos, FlownDirection(state, lut, u), 10.0f);
    }

    // a tick across the path for each second of travel, longer every fifth second
    if (state.speed > 0.0f) {
        graphics.SetColor(moth::gfx::FromARGB(0xFFFFFFFF));
        for (int second = 1; static_cast<float>(second) * state.speed < totalLength; ++second) {
            auto const u = (static_cast<float>(second) * state.speed) / totalLength;
            auto const pos = FlownAt(state, lut, u) + offset;
            auto const direction = FlownDirection(state, lut, u);
            auto const across = moth::gfx::FloatVec2{ -direction.y, direction.x };
            auto const size = second % 5 == 0 ? 10.0f : 5.0f;
            graphics.DrawLineF(pos - (across * size), pos + (across * size));
        }
    }

    // the ghost, and the way it is facing
    auto const ghost = FlownAt(state, lut, std::clamp(progress, 0.0f, 1.0f)) + offset;
    auto const ghostDirection = FlownDirection(state, lut, std::clamp(progress, 0.0f, 1.0f));
    graphics.SetColor(moth::gfx::FromARGB(0xFFFFFFFF));
    float const ghostSize = 8.0f;
    auto const across = moth::gfx::FloatVec2{ -ghostDirection.y, ghostDirection.x };
    graphics.DrawLineF(ghost + (ghostDirection * ghostSize), ghost + (across * ghostSize));
    graphics.DrawLineF(ghost + (across * ghostSize), ghost - (ghostDirection * ghostSize));
    graphics.DrawLineF(ghost - (ghostDirection * ghostSize), ghost - (across * ghostSize));
    graphics.DrawLineF(ghost - (across * ghostSize), ghost + (ghostDirection * ghostSize));
    graphics.DrawLineF(ghost, ghost + (ghostDirection * (ghostSize * 2.0f)));
}
