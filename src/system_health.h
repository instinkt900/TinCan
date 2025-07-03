#pragma once

#include <entt/entity/fwd.hpp>
#include <functional>

struct ComponentHealth {
    int32_t m_currentHealth = 100;
    int32_t m_maxHealth = 100;
    bool m_dead = false;
    entt::entity m_lastDamager;
    std::function<void(entt::entity, entt::entity)> m_onDeath;
};

class SystemHealth {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
};
