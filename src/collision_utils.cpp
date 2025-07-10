#include "collision_utils.h"
#include <moth_ui/utils/vector_utils.h>

float SweepTest(moth_ui::FloatVec2 const& obj1Pos, moth_ui::FloatVec2 const& obj1LastPos, float obj1Radius,
                moth_ui::FloatVec2 const& obj2Pos, moth_ui::FloatVec2 const& obj2LastPos, float obj2Radius) {
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
