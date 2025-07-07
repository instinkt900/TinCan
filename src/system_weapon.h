#pragma once

#include "projectile_database.h"
#include <entt/entity/fwd.hpp>
#include <canyon/graphics/surface_context.h>
#include <moth_ui/utils/vector.h>

struct ComponentWeapon {
    bool m_active = false;
    int32_t m_cooldown = 0;
    int32_t m_maxCooldown = 1000;
    bool m_playerTracking = false;
    float m_angle = 0;
    std::string m_projectileName;
};

class SystemWeapon {
public:
    static void Update(entt::registry& registry, uint32_t ticks, ProjectileDatabase& projectileDatabase);
};
