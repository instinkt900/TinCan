#pragma once

#include "system_projectile.h"
#include <entt/entity/fwd.hpp>
#include <canyon/graphics/surface_context.h>
#include <moth_ui/utils/vector.h>

struct ProjectileTemplate {
    std::shared_ptr<canyon::graphics::IImage> m_sprite;
    moth_ui::FloatVec2 m_spriteSize = { 0, 0 };
    Team m_team;
    ProjectileColor m_color;
    int32_t m_damage = 1;
    float m_speed = 1000.0f;
    uint32_t m_lifetime = 1000;
    float m_radius = 0;
};

struct ComponentWeapon {
    bool m_active = false;
    uint32_t m_cooldown = 0;
    uint32_t m_maxCooldown = 1000;
    ProjectileTemplate m_projectileTemplate;
};

class SystemWeapon {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
};
