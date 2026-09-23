#include "system_input.h"
#include "component_entity.h"
#include "game_world.h"
#include "gamedata_input.h"
#include "system_power_weapon.h"
#include "system_weapon.h"
#include <moth/ecs/world.h>

void SystemInput::OnKey(moth::ecs::World& registry, InputData const& bindings,
                        moth::ui::EventKey const& event) {
    auto const& actions = bindings.Actions(event.GetKey());
    if (actions.empty()) {
        return;
    }

    bool const down = event.GetAction() == moth::ui::KeyAction::Down;
    auto view = registry.View<ComponentInput>();

    for (auto [entity, input] : view.each()) {
        for (auto const action : actions) {
            auto const index = static_cast<size_t>(action);
            input.m_held[index] = down;
            if (down) {
                input.m_pressed[index] = true;
            }
        }
    }
}

void SystemInput::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto view = registry.View<ComponentInput>();

    for (auto [entity, input] : view.each()) {
        if (auto* weapons = registry.TryGet<ComponentWeapon>(entity)) {
            for (auto& weapon : weapons->m_weapons) {
                weapon.m_active = input.Held(InputAction::Fire);
            }
        }

        if (auto* weapon = registry.TryGet<ComponentPowerWeapon>(entity)) {
            weapon->m_activated = input.Held(InputAction::PowerWeapon);
        }

        if (auto* entityData = registry.TryGet<ComponentEntity>(entity)) {
            if (input.Pressed(InputAction::SwapAffinity)) {
                entityData->m_affinity =
                    entityData->m_affinity == Affinity::Light ? Affinity::Dark : Affinity::Light;
            }
        }
    }
}

void SystemInput::EndFrame(GameWorld& world) {
    auto view = world.GetRegistry().View<ComponentInput>();

    for (auto [entity, input] : view.each()) {
        input.m_pressed.fill(false);
    }
}
