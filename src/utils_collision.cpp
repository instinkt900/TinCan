#include "utils_collision.h"
#include <moth_ui/utils/vector_utils.h>
#include <vector>

namespace {
    bool collisionSweepMovingStatic(canyon::FloatVec2 const& circle1, canyon::FloatVec2 const& velocity,
                                    float circle1r, canyon::FloatVec2 const& circle2, float circle2r,
                                    float* out_t = nullptr) {
        auto const rel = circle2 - circle1;
        auto const combined_r = circle1r + circle2r;
        auto const combined_r2 = combined_r * combined_r;

        auto const a = Dot(velocity, velocity);
        if (a == 0.0f) {
            // Circle isn't moving; do a static overlap test
            return Dot(rel, rel) <= combined_r2;
        }

        auto const b = 2.0f * Dot(velocity, rel);
        auto const c = Dot(rel, rel) - combined_r2;

        auto const disc = (b * b) - (4.0f * a * c);
        if (disc < 0.0f) {
            return false; // No intersection
        }

        auto const sqrt_disc = std::sqrt(disc);
        auto const t1 = (-b - sqrt_disc) / (2.0f * a);
        auto const t2 = (-b + sqrt_disc) / (2.0f * a);

        // We only care about the first positive time of impact in [0, 1]
        if (t1 >= 0.0f && t1 <= 1.0f) {
            if (out_t != nullptr) {
                *out_t = t1;
            }
            return true;
        }
        if (t2 >= 0.0f && t2 <= 1.0f) {
            if (out_t != nullptr) {
                *out_t = t2;
            }
            return true;
        }

        return false;
    }
}

float SweepTest(canyon::FloatVec2 const& obj1Pos, canyon::FloatVec2 const& obj1LastPos, float obj1Radius,
                canyon::FloatVec2 const& obj2Pos, canyon::FloatVec2 const& obj2LastPos, float obj2Radius) {
    auto const vel1 = obj1Pos - obj1LastPos;
    auto const vel2 = obj2Pos - obj2LastPos;
    auto const relativeVel = vel2 - vel1;

    float t = 0;
    if (collisionSweepMovingStatic(obj1Pos, relativeVel, obj1Radius, obj2Pos, obj2Radius, &t)) {
        return t;
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
