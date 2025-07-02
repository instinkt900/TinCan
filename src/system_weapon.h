#pragma once

#include <entt/entity/fwd.hpp>
#include <canyon/graphics/surface_context.h>
#include <moth_ui/utils/vector.h>

enum BulletColor {
    WHITE,
    BLACK,
};

struct ProjectileTemplate {
    std::shared_ptr<canyon::graphics::IImage> m_sprite;
    moth_ui::FloatVec2 m_spriteSize = { 0, 0 };
    float m_speed = 1000.0f;
    uint32_t m_lifetime = 1000;
};

struct ComponentWeapon {
    bool m_active = false;
    bool m_player = false;
    BulletColor m_color = BulletColor::WHITE;
    uint32_t m_cooldown = 0;
    uint32_t m_maxCooldown = 1000;
    ProjectileTemplate m_projectile;
};

struct ComponentBullet {
    bool m_player = false;
    BulletColor m_color = BulletColor::WHITE;
};

class SystemWeapon {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
};
