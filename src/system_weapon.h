#pragma once

#include <entt/entity/fwd.hpp>
#include <canyon/graphics/surface_context.h>
#include <functional>
#include <moth_ui/utils/vector.h>

enum BulletColor {
    WHITE,
    BLACK,
};

struct ProjectileTemplate {
    std::shared_ptr<canyon::graphics::IImage> m_sprite;
    moth_ui::FloatVec2 m_spriteSize = { 0, 0 };
    int32_t m_damage = 1;
    float m_speed = 1000.0f;
    uint32_t m_lifetime = 1000;
    float m_radius = 0;
    uint32_t m_collisionMask = 0;
    uint32_t m_collidesWithMask = 0;
    std::function<void(entt::entity, entt::entity)> m_onCollision;
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
    int32_t m_damage = 1;
};

class SystemWeapon {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
};
