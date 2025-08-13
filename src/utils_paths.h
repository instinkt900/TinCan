#pragma once

#include <canyon/utils/math_utils.h>
#include <canyon/utils/vector.h>

inline canyon::FloatVec2 CatmullRom(canyon::FloatVec2 const& p0, canyon::FloatVec2 const& p1,
                                    canyon::FloatVec2 const& p2, canyon::FloatVec2 const& p3, float t) {
    auto const t2 = t * t;
    auto const t3 = t2 * t;
    return 0.5f * ((2.0f * p1) + (-p0 + p2) * t + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

inline canyon::FloatVec2 SampleCatmullRomPath(std::vector<canyon::FloatVec2> const& points, float t) {
    if (t >= 1.0f) {
        return points.back();
    }

    auto const n = static_cast<int>(points.size());
    if (n < 4) {
        return points[0];
    }

    auto const segmentF = t * static_cast<float>(n - 3);
    auto const i = std::min(static_cast<int>(segmentF), n - 4);
    auto const localT = segmentF - static_cast<float>(i);

    return CatmullRom(points[i], points[i + 1], points[i + 2], points[i + 3], localT);
}

struct ArcLengthLUT {
    std::vector<float> distances; // length at sample i
    std::vector<float> t_values;  // corresponding t values
};

using SplineDefinition = std::vector<canyon::FloatVec2>;

inline ArcLengthLUT BuildArcLengthTable(SplineDefinition const& spline, int samples = 1000) {
    ArcLengthLUT table;
    table.t_values.reserve(samples);
    table.distances.reserve(samples);

    canyon::FloatVec2 prev = SampleCatmullRomPath(spline, 0.0f);
    float dist_accum = 0.0f;
    for (int i = 0; i <= samples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(samples);
        canyon::FloatVec2 curr = SampleCatmullRomPath(spline, t);
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
    float f = (s - d0) / (d1 - d0);
    return canyon::Lerp(t0, t1, f);
}

inline canyon::FloatVec2 SamplePathByDistance(SplineDefinition const& spline, ArcLengthLUT const& table, float distance) {
    float t = GetTForDistance(table, distance);
    return SampleCatmullRomPath(spline, t);
}

