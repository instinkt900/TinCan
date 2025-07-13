#pragma once

#include <entt/entity/entity.hpp>

struct ComponentHealth {
    float m_currentHealth = 100;
    float m_maxHealth = 100;
    bool m_dead = false;
    entt::entity m_lastDamager = entt::null;
};
