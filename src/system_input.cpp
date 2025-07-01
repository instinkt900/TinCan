#include "system_input.h"
#include "system_weapon.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

void SystemInput::OnKey(entt::registry& registry, moth_ui::EventKey const& event) {
    auto view = registry.view<ComponentInput>();

    for (auto [entity, input] : view.each()) {
        input.m_state[event.GetKey()] = event.GetAction() == moth_ui::KeyAction::Down;

        if (auto* weapon = registry.try_get<ComponentWeapon>(entity)) {
            weapon->m_active = input.m_state[moth_ui::Key::Space];
        }
    }
}
