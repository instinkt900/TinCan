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

// How a path runs between its waypoints. Both are sampled by arc length, so speed means the same thing
// whichever is chosen.
enum class PathInterpolation {
    Linear,
    CatmullRom,
};

// Samples a polyline through the points. Lengths shorter than two points match SampleCatmullRomPath.
inline moth::gfx::FloatVec2 SampleLinearPath(std::vector<moth::gfx::FloatVec2> const& points, float t) {
    if (points.empty()) {
        return { 0, 0 };
    }
    if (points.size() == 1) {
        return points[0];
    }

    t = std::clamp(t, 0.0f, 1.0f);
    auto const n = static_cast<int>(points.size());
    auto const segmentF = t * static_cast<float>(n - 1);
    auto const i = std::min(static_cast<int>(segmentF), n - 2);
    auto const localT = segmentF - static_cast<float>(i);

    return points[i] + (points[i + 1] - points[i]) * localT;
}

inline moth::gfx::FloatVec2 SamplePath(std::vector<moth::gfx::FloatVec2> const& points,
                                       PathInterpolation interpolation, float t) {
    return interpolation == PathInterpolation::CatmullRom ? SampleCatmullRomPath(points, t)
                                                          : SampleLinearPath(points, t);
}

// How an entity gets from wherever it is to the start of a path.
enum class TransitionType {
    // appear at the start of the path, which is what happens when nothing is asked for
    Snap,
    // a straight line to it
    Linear,
    // a curve that leaves along the way the entity was already going and arrives along the way the path
    // sets off, so the join cannot be seen
    Spline,
};

// A cubic through p0 and p1 that leaves with velocity m0 and arrives with velocity m1.
inline moth::gfx::FloatVec2 Hermite(moth::gfx::FloatVec2 const& p0, moth::gfx::FloatVec2 const& m0,
                                    moth::gfx::FloatVec2 const& p1, moth::gfx::FloatVec2 const& m1,
                                    float t) {
    auto const t2 = t * t;
    auto const t3 = t2 * t;
    auto const h00 = (2.0f * t3) - (3.0f * t2) + 1.0f;
    auto const h10 = t3 - (2.0f * t2) + t;
    auto const h01 = (-2.0f * t3) + (3.0f * t2);
    auto const h11 = t3 - t2;
    return (p0 * h00) + (m0 * h10) + (p1 * h01) + (m1 * h11);
}

// The approach into the start of a path, as a list of points. A curve is sampled into a polyline so that
// the same arc-length sampler that carries an entity along a path carries it along the approach too, which
// is what makes the speed mean the same thing in both. A direction that is zero, because the entity was
// holding still or because there is no path behind it, falls back to pointing straight at the target.
inline std::vector<moth::gfx::FloatVec2> BuildTransitionPath(TransitionType type,
                                                             moth::gfx::FloatVec2 const& from,
                                                             moth::gfx::FloatVec2 const& fromDirection,
                                                             moth::gfx::FloatVec2 const& to,
                                                             moth::gfx::FloatVec2 const& toDirection) {
    auto const chord = to - from;
    if (type != TransitionType::Spline || moth::gfx::LengthSq(chord) <= 0.0f) {
        return { from, to };
    }

    auto const straight = moth::gfx::Normalized(chord);
    auto const length = moth::gfx::Length(chord);
    auto const m0 = (moth::gfx::LengthSq(fromDirection) > 0.0f ? fromDirection : straight) * length;
    auto const m1 = (moth::gfx::LengthSq(toDirection) > 0.0f ? toDirection : straight) * length;

    int const samples = 24;
    std::vector<moth::gfx::FloatVec2> points;
    points.reserve(samples + 1);
    for (int i = 0; i <= samples; ++i) {
        points.push_back(Hermite(from, m0, to, m1, static_cast<float>(i) / static_cast<float>(samples)));
    }
    return points;
}

struct ArcLengthLUT {
    std::vector<float> distances; // length at sample i
    std::vector<float> t_values;  // corresponding t values
};

using SplineDefinition = std::vector<moth::gfx::FloatVec2>;

inline ArcLengthLUT BuildArcLengthTable(SplineDefinition const& spline, PathInterpolation interpolation,
                                        int samples = 1000) {
    ArcLengthLUT table;
    table.t_values.reserve(samples);
    table.distances.reserve(samples);

    moth::gfx::FloatVec2 prev = SamplePath(spline, interpolation, 0.0f);
    float dist_accum = 0.0f;
    for (int i = 0; i <= samples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(samples);
        moth::gfx::FloatVec2 curr = SamplePath(spline, interpolation, t);
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

inline moth::gfx::FloatVec2 SamplePathByDistance(SplineDefinition const& spline,
                                                 PathInterpolation interpolation, ArcLengthLUT const& table,
                                                 float distance) {
    float t = GetTForDistance(table, distance);
    return SamplePath(spline, interpolation, t);
}

// The direction of travel at a point on the path, found by stepping along the path's arc length rather
// than along its parameter. The two are only the same when the path happens to be evenly parameterised,
// as a straight line between two waypoints is. On a curve they can be most of the path apart.
inline moth::gfx::FloatVec2 PathTangentAtDistance(SplineDefinition const& spline,
                                                  PathInterpolation interpolation,
                                                  ArcLengthLUT const& table, float distance) {
    auto const totalLength = table.distances.back();
    if (totalLength <= 0.0f) {
        return { 0, 0 };
    }
    auto const step = totalLength * 0.002f;
    auto const ahead =
        SamplePathByDistance(spline, interpolation, table, std::min(totalLength, distance + step));
    auto const behind = SamplePathByDistance(spline, interpolation, table, std::max(0.0f, distance - step));
    auto const delta = ahead - behind;
    if (moth::gfx::LengthSq(delta) <= 0.0f) {
        return { 0, 0 };
    }
    return moth::gfx::Normalized(delta);
}

