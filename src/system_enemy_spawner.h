#pragma once

#include "system_projectile.h"
#include "system_weapon.h"
#include <canyon/graphics/iimage.h>
#include <entt/entity/fwd.hpp>
#include <functional>
#include <moth_ui/utils/vector.h>
#include <moth_ui/utils/rect.h>

struct EnemyTemplate {
    ComponentDrawable m_drawable;
    Team m_team = Team::NONE;
    EnergyColor m_color = EnergyColor::WHITE;
    float m_radius = 1;
    int32_t m_maxHealth = 100;
    float m_speed = 1.0f;
    uint32_t m_lifetime = 1000;
    ComponentWeapon m_weapon;
    std::function<void(entt::entity, entt::entity)> m_onDeath;
};

struct ComponentEnemySpawner {
    bool m_active = false;
    uint32_t m_cooldown = 0;
    uint32_t m_maxCooldown = 1000;
    moth_ui::IntRect m_spawnRegion;
    EnemyTemplate m_enemyTemplate;
};

class SystemEnemySpawner {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
};
