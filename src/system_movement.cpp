#include "system_movement.h"
#include "system_input.h"
#include <entt/entt.hpp>

void SystemMovement::Update(entt::registry& registry, uint32_t ticks) {
    auto view = registry.view<ComponentPosition, ComponentVelocity>();

    float const dt = static_cast<float>(ticks) / 1000.0f;
    for (auto [entity, pos, vel]: view.each()) {
        auto velocity = vel.m_velocity;
        if (auto* input = registry.try_get<ComponentInput>(entity)) {
            if (input->m_state[moth_ui::Key::Up]) {
                velocity.y -= 300.0f;
            }
            if (input->m_state[moth_ui::Key::Down]) {
                velocity.y += 300.0f;
            }
            if (input->m_state[moth_ui::Key::Left]) {
                velocity.x -= 300.0f;
            }
            if (input->m_state[moth_ui::Key::Right]) {
                velocity.x += 300.0f;
            }

        }
        pos.m_position += velocity * dt;
    }
}

