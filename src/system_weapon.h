#pragma once

#include <entt/entity/fwd.hpp>
#include <canyon/graphics/surface_context.h>

enum BulletColor {
    WHITE,
    BLACK,
};

struct ComponentWeapon {
    bool m_active;
    bool m_player;
    BulletColor m_color;
    uint32_t m_cooldown;
    uint32_t m_maxCooldown;
};

struct ComponentBullet {
    bool m_player;
    BulletColor m_color;
};

class SystemWeapon {
public:
    static void Update(entt::registry& registry, uint32_t ticks, canyon::graphics::SurfaceContext& surfaceContext);
};
