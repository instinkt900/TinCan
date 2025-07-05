#include "collision_utils.h"
#include <moth_ui/utils/vector_utils.h>

float SweepTest(moth_ui::FloatVec2 const& stationary, moth_ui::FloatVec2 const& moving,
                moth_ui::FloatVec2 const& velocity, float radius) {
    auto const delta = moving - stationary;

    float const a = moth_ui::Dot(velocity, velocity);
    float const b = 2.0f * moth_ui::Dot(delta, velocity);
    float const c = moth_ui::Dot(delta, delta) - (radius * radius);

    if (a == 0.0f) {
        if (moth_ui::DistanceSq(stationary, moving) <= (radius * radius)) {
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
