#pragma once

#include <moth/ecs/world.h>

struct ComponentHealth {
    float m_maxHealth = 100;
    float m_currentHealth = 100;
    bool m_dead = false;
    moth::ecs::Entity m_lastDamager = moth::ecs::kNullEntity;

    explicit ComponentHealth(float maxHealth)
        : m_maxHealth(maxHealth)
        , m_currentHealth(maxHealth) {}
};
