#pragma once

#include "system_drawable.h"
#include "system_projectile.h"
#include <entt/entity/fwd.hpp>
#include <canyon/graphics/surface_context.h>
#include <moth_ui/utils/vector.h>

struct ProjectileTemplate {
    ComponentDrawable m_drawable{};
    Team m_team = Team::NONE;
    int32_t m_damage = 1;
    float m_speed = 1000.0f;
    uint32_t m_lifetime = 1000;
    float m_radius = 0;
};

struct ComponentWeapon {
    bool m_active = false;
    uint32_t m_cooldown = 0;
    uint32_t m_maxCooldown = 1000;
    bool m_playerTracking = false;
    ProjectileTemplate m_projectileTemplate;
};

class SystemWeapon {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
};
