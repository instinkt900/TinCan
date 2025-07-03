#pragma once

#include "system_weapon.h"
#include <canyon/graphics/iimage.h>
#include <entt/entity/fwd.hpp>
#include <moth_ui/utils/vector.h>
#include <moth_ui/utils/rect.h>

struct EnemyTemplate {
    std::shared_ptr<canyon::graphics::IImage> m_sprite;
    moth_ui::FloatVec2 m_spriteSize = { 0, 0 };
    float m_speed;
    uint32_t m_lifetime = 1000;
    ComponentWeapon m_weapon;
};

struct ComponentEnemySpawner {
    bool m_active = false;
    uint32_t m_cooldown = 0;
    uint32_t m_maxCooldown = 1000;
    moth_ui::IntRect m_spawnRegion;
    EnemyTemplate m_template;
};

struct ComponentEnemy {
    int m_health = 100;
};

class SystemEnemySpawner {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
};
