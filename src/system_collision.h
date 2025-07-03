#pragma once

#include <entt/entity/fwd.hpp>
#include <functional>

enum CollisionGroups {
    PLAYER = 1 << 0,
    ENEMY = 1 << 1,
};

struct ComponentCollision {
    float m_radius;
    uint32_t m_collisionMask;
    uint32_t m_collidesWithMask;
    std::function<void(entt::entity, entt::entity)> m_onCollision;
};

class SystemCollision {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
};
