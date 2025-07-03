#include "system_collision.h"
#include "system_movement.h"
#include <entt/entt.hpp>
#include <moth_ui/utils/vector_utils.h>
#include <spdlog/spdlog.h>

float Sweep(moth_ui::FloatVec2 const& stationary, moth_ui::FloatVec2 const& moving, moth_ui::FloatVec2 const& velocity, float radius) {
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

void SystemCollision::Update(entt::registry& registry, uint32_t ticks) {
    auto view1 = registry.view<ComponentCollision, ComponentPosition, ComponentVelocity>();
    for (auto [entity1, collision1, position1, velocity1] : view1.each()) {
        auto view2 = registry.view<ComponentCollision, ComponentPosition, ComponentVelocity>();
        for (auto [entity2, collision2, position2, velocity2] : view2.each()) {
            if (entity1 == entity2) {
                continue;
            }
            if ((collision1.m_collidesWithMask & collision2.m_collisionMask) != 0) {
                auto const entity1Vel = position1.m_position - position1.m_lastPosition;
                auto const entity2Vel = position2.m_position - position2.m_lastPosition;
                auto const relativeVel = entity2Vel - entity1Vel;
                auto const combinedRadius = collision1.m_radius + collision2.m_radius;
                auto const t = Sweep(position1.m_position, position2.m_lastPosition, relativeVel, combinedRadius);
                if (t < 1.0f) {
                    collision1.m_onCollision(entity1, entity2);
                }
            }
        }
    }
}
