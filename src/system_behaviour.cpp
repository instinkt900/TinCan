#include "system_behaviour.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

void BehaviourStraight(entt::entity entity, ComponentBehaviour& behaviour,
                                     ComponentPosition& position, uint32_t ticks) {
    auto parameterSpeed = behaviour.m_parameters.find("speed");
    if (parameterSpeed == behaviour.m_parameters.end()) {
        return;
    }
    float const speed = std::get<float>(parameterSpeed->second);
    float const t = static_cast<float>(ticks) / 1000.0f;
    auto const newPosition = behaviour.m_offset + moth_ui::FloatVec2{ 0, t * speed };
    position.m_lastPosition = position.m_position;
    position.m_position = newPosition;
}

SystemBehaviour::SystemBehaviour() {
    m_behaviours["straight"] = BehaviourStraight;
}

void SystemBehaviour::Update(entt::registry& registry, uint32_t ticks) {
    auto view = registry.view<ComponentBehaviour, ComponentPosition, ComponentLifetime>();

    for (auto [entity, behaviour, position, lifetime] : view.each()) {
        auto entry = m_behaviours.find(behaviour.m_behaviourName);
        if (entry == m_behaviours.end()) {
            spdlog::error("Unknown behaviour name {}", behaviour.m_behaviourName);
            continue;
        }
        entry->second(entity, behaviour, position, lifetime.m_msAlive);
    }
}
