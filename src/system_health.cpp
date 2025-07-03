#include "system_health.h"
#include <entt/entt.hpp>

void SystemHealth::Update(entt::registry& registry, uint32_t ticks) {
    auto view = registry.view<ComponentHealth>();

    for (auto [entity, health]: view.each()) {
        if (!health.m_dead && health.m_currentHealth <= 0) {
            health.m_dead = true;
            if (health.m_onDeath) {
                health.m_onDeath(entity, health.m_lastDamager);
            }
        }
    }
}

