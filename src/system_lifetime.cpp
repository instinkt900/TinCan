#include "system_lifetime.h"
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

void SystemLifetime::Update(entt::registry& registry, uint32_t ticks) {
    std::list<entt::entity> deadEntities;

    auto view = registry.view<ComponentLifetime>();
    for (auto [entity, lifetime] : view.each()) {
        if (lifetime.m_msLeft <= ticks) {
            deadEntities.push_back(entity);
        } else {
            lifetime.m_msLeft -= ticks;
            lifetime.m_msAlive += ticks;
        }
    }

    for (auto entity : deadEntities) {
        registry.destroy(entity);
    }
}
