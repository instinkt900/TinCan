#include "system_collision.h"
#include "system_movement.h"
#include <entt/entt.hpp>
#include <moth_ui/utils/vector_utils.h>
#include <spdlog/spdlog.h>

void SystemCollision::Update(entt::registry& registry, uint32_t ticks) {
    auto view1 = registry.view<ComponentCollision, ComponentPosition>();
    for (auto [entity1, collision1, position1] : view1.each()) {
        auto view2 = registry.view<ComponentCollision, ComponentPosition>();
        for (auto [entity2, collision2, position2] : view2.each()) {
            if (entity1 == entity2) {
                continue;
            }
            if ((collision1.m_collidesWithMask & collision2.m_collisionMask) != 0) {
                float const combinedRadius = collision1.m_radius + collision2.m_radius;
                float const distance = moth_ui::Distance(position1.m_position, position2.m_position);
                if (distance <= combinedRadius && collision1.m_onCollision) {
                    collision1.m_onCollision(entity1, entity2);
                }
            }
        }
    }
}
