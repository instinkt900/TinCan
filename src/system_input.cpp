#include "system_input.h"
#include "component_entity.h"
#include "system_drawable.h"
#include "system_power_weapon.h"
#include "system_weapon.h"
#include <moth/ecs/world.h>
#include <spdlog/spdlog.h>

void SystemInput::OnKey(moth::ecs::World& registry, moth::ui::EventKey const& event) {
    auto view = registry.View<ComponentInput>();

    for (auto [entity, input] : view.each()) {
        input.m_lastState = input.m_state;
        input.m_state[event.GetKey()] = event.GetAction() == moth::ui::KeyAction::Down;

        // TODO: remove this temp test
        auto cameraView = registry.View<ComponentCamera>();
        if (!cameraView.empty()) {
            if (!input.m_lastState[moth::ui::Key::S] && input.m_state[moth::ui::Key::S]) {
                auto& camera = cameraView.get<ComponentCamera>(*cameraView.begin());
                camera.m_shake.emplace_back(CameraShake(4.0f, 1000));
                spdlog::info("shake");
            }
        }

        if (auto* weapon = registry.TryGet<ComponentWeapon>(entity)) {
            weapon->m_active = input.m_state[moth::ui::Key::Space];
        }

        if (auto* weapon = registry.TryGet<ComponentPowerWeapon>(entity)) {
            weapon->m_activated = input.m_state[moth::ui::Key::Z];
        }

        if (auto* entityData = registry.TryGet<ComponentEntity>(entity)) {
            if (!input.m_lastState[moth::ui::Key::X] && input.m_state[moth::ui::Key::X]) {
                entityData->m_affinity =
                    entityData->m_affinity == Affinity::Light ? Affinity::Dark : Affinity::Light;
            }
        }
    }
}
