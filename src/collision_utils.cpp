#include "collision_utils.h"
#include <moth_ui/utils/vector_utils.h>
#include <vector>

float SweepTest(canyon::FloatVec2 const& obj1Pos, canyon::FloatVec2 const& obj1LastPos, float obj1Radius,
                canyon::FloatVec2 const& obj2Pos, canyon::FloatVec2 const& obj2LastPos, float obj2Radius) {
    auto const vel1 = obj1Pos - obj2LastPos;
    auto const vel2 = obj2Pos - obj2LastPos;
    auto const relativeVel = vel2 - vel1;
    auto const combinedRadius = obj1Radius + obj2Radius;

    auto const delta = obj2LastPos - obj1Pos;

    float const a = moth_ui::Dot(relativeVel, relativeVel);
    float const b = 2.0f * moth_ui::Dot(delta, relativeVel);
    float const c = moth_ui::Dot(delta, delta) - (combinedRadius * combinedRadius);

    if (a == 0.0f) {
        if (moth_ui::DistanceSq(obj1Pos, obj2Pos) <= (combinedRadius * combinedRadius)) {
            return 0.0f;
        }
        return 1.0f;
    }

    float const disc = (b * b) - (4 * a * c);
    if (disc < 0.0f) {
        return 1.0f;
    }

    float const sqrtDist = std::sqrt(disc);
    float const t1 = (-b - sqrtDist) / (2.0f * a);
    float const t2 = (-b + sqrtDist) / (2.0f * a);

    if (t1 >= 0.0f && t1 <= 1.0f) {
        return t1;
    }

    if (t2 >= 0.0f && t2 <= 1.0f) {
        return t2;
    }

    return 1.0f;
}

bool IntersectSegmentWithLine(canyon::FloatVec2 const& p1, canyon::FloatVec2 const& p2,
                              canyon::FloatVec2 const& a, canyon::FloatVec2 const& b,
                              canyon::FloatVec2& out) {
    auto const r = p2 - p1;
    auto const s = b - a;
    float denom = (r.x * s.y) - (r.y * s.x);
    if (std::abs(denom) < 1e-6f) {
        return false; // parallel
    }

    auto t = ((a - p1).x * s.y - (a - p1).y * s.x) / denom;
    auto u = ((a - p1).x * r.y - (a - p1).y * r.x) / denom;

    if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
        out = p1 + (r * t);
        return true;
    }
    return false;
}

bool GetExitPoint(canyon::FloatVec2 p1, canyon::FloatVec2 p2, const canyon::FloatRect& bounds,
                  canyon::FloatVec2& out) {
    std::vector<std::pair<canyon::FloatVec2, canyon::FloatVec2>> edges = {
        { { bounds.left(), bounds.top() }, { bounds.right(), bounds.top() } },        // top
        { { bounds.right(), bounds.top() }, { bounds.right(), bounds.bottom() } },    // right
        { { bounds.right(), bounds.bottom() }, { bounds.right(), bounds.bottom() } }, // bottom
        { { bounds.left(), bounds.bottom() }, { bounds.left(), bounds.top() } }       // left
    };

    for (auto& [a, b] : edges) {
        if (IntersectSegmentWithLine(p1, p2, a, b, out)) {
            return true;
        }
    }

    return false;
}
