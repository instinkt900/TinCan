#include "system_world_bounds.h"
#include "system_movement.h"
#include "tags.h"
#include <entt/entt.hpp>

void SystemWorldBounds::Update(entt::registry& registry, uint32_t ticks, canyon::FloatRect const& bounds) {
    auto view = registry.view<ComponentPosition, BoundedTag>();

    for (auto [entity, position] : view.each()) {
        if (!moth_ui::IsInRect(position.m_position, bounds)) {
            auto delta = position.m_position - position.m_lastPosition;
            if (position.m_position.x < bounds.x() || position.m_position.x > (bounds.x() + bounds.w())) {
                delta.x = 0;
            }
            if (position.m_position.y < bounds.y() || position.m_position.y > (bounds.y() + bounds.h())) {
                delta.y = 0;
            }
            position.m_position = position.m_lastPosition + delta;
        }
    }
}
