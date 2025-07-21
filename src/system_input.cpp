#include "system_input.h"
#include "component_entity.h"
#include "system_power_weapon.h"
#include "system_weapon.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

void SystemInput::OnKey(entt::registry& registry, moth_ui::EventKey const& event) {
    auto view = registry.view<ComponentInput>();

    for (auto [entity, input] : view.each()) {
        input.m_lastState = input.m_state;
        input.m_state[event.GetKey()] = event.GetAction() == moth_ui::KeyAction::Down;

        if (auto* weapon = registry.try_get<ComponentWeapon>(entity)) {
            weapon->m_active = input.m_state[moth_ui::Key::Space];
        }

        if (auto* weapon = registry.try_get<ComponentPowerWeapon>(entity)) {
            weapon->m_activated = input.m_state[moth_ui::Key::Z];
        }

        if (auto* entityData = registry.try_get<ComponentEntity>(entity)) {
            if (!input.m_lastState[moth_ui::Key::X] && input.m_state[moth_ui::Key::X]) {
                entityData->m_color =
                    entityData->m_color == EnergyColor::Blue ? EnergyColor::Red : EnergyColor::Blue;
            }
        }
    }
}
