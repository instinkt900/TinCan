#pragma once

#include <moth/graphics/utils/math_utils.h>
#include <moth/graphics/utils/vector.h>
#include <algorithm>
#include <vector>

inline moth::gfx::FloatVec2 CatmullRom(moth::gfx::FloatVec2 const& p0, moth::gfx::FloatVec2 const& p1,
                                    moth::gfx::FloatVec2 const& p2, moth::gfx::FloatVec2 const& p3, float t) {
    auto const t2 = t * t;
    auto const t3 = t2 * t;
    return 0.5f * ((2.0f * p1) + (-p0 + p2) * t + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

// The control points either side of a segment. Indices outside the list clamp to its ends, which is the
// endpoint padding a Catmull-Rom spline needs. Doing it here rather than by duplicating points in the data
// keeps an authored path and a loaded one the same path.
inline moth::gfx::FloatVec2 PathPointAt(std::vector<moth::gfx::FloatVec2> const& points, int index) {
    auto const last = static_cast<int>(points.size()) - 1;
    return points[static_cast<size_t>(std::clamp(index, 0, last))];
}

// Samples a path of any length. An empty path is the origin, one point is that point, two points are the
// straight line between them, and three or more are a Catmull-Rom spline through all of them.
inline moth::gfx::FloatVec2 SampleCatmullRomPath(std::vector<moth::gfx::FloatVec2> const& points, float t) {
    if (points.empty()) {
        return { 0, 0 };
    }
    if (points.size() == 1) {
        return points[0];
    }

    t = std::clamp(t, 0.0f, 1.0f);
    if (points.size() == 2) {
        return points[0] + (points[1] - points[0]) * t;
    }

    auto const n = static_cast<int>(points.size());
    auto const segmentF = t * static_cast<float>(n - 1);
    auto const i = std::min(static_cast<int>(segmentF), n - 2);
    auto const localT = segmentF - static_cast<float>(i);

    return CatmullRom(PathPointAt(points, i - 1), PathPointAt(points, i), PathPointAt(points, i + 1),
                      PathPointAt(points, i + 2), localT);
}

struct ArcLengthLUT {
    std::vector<float> distances; // length at sample i
    std::vector<float> t_values;  // corresponding t values
};

using SplineDefinition = std::vector<moth::gfx::FloatVec2>;

inline ArcLengthLUT BuildArcLengthTable(SplineDefinition const& spline, int samples = 1000) {
    ArcLengthLUT table;
    table.t_values.reserve(samples);
    table.distances.reserve(samples);

    moth::gfx::FloatVec2 prev = SampleCatmullRomPath(spline, 0.0f);
    float dist_accum = 0.0f;
    for (int i = 0; i <= samples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(samples);
        moth::gfx::FloatVec2 curr = SampleCatmullRomPath(spline, t);
        if (i > 0) {
            dist_accum += Length(curr - prev);
        }
        table.t_values.push_back(t);
        table.distances.push_back(dist_accum);
        prev = curr;
    }
    return table;
}

inline float GetTForDistance(ArcLengthLUT const& table, float s) {
    // Binary search
    size_t low = 0;
    size_t high = table.distances.size() - 1;
    while (low < high) {
        size_t mid = (low + high) / 2;
        if (table.distances[mid] < s) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    // Linear interpolate between [low-1, low]
    if (low == 0) {
        return table.t_values[0];
    }

    float d0 = table.distances[low - 1];
    float d1 = table.distances[low];
    float t0 = table.t_values[low - 1];
    float t1 = table.t_values[low];
    if (d1 <= d0) {
        // a path that covers no distance, so there is nothing to interpolate along
        return t1;
    }
    float f = (s - d0) / (d1 - d0);
    return moth::gfx::Lerp(t0, t1, f);
}

inline moth::gfx::FloatVec2 SamplePathByDistance(SplineDefinition const& spline, ArcLengthLUT const& table, float distance) {
    float t = GetTForDistance(table, distance);
    return SampleCatmullRomPath(spline, t);
}

