#pragma once

#include "gamedata_projectile.h"
#include <moth/ecs/world.h>
#include <moth/graphics/utils/vector.h>

class GameWorld;

struct ComponentProjectile {
    moth::ecs::Entity m_source = moth::ecs::kNullEntity;
    float m_damage = 1;
};

class SystemProjectile {
public:
    static moth::ecs::Entity CreateProjectile(moth::ecs::World& registry, ProjectileData const& data,
                                         moth::ecs::Entity source, moth::gfx::FloatVec2 const& position,
                                         moth::gfx::FloatVec2 const& direction, float speed, float rotation,
                                         float damage);
    static void Update(GameWorld& world, uint32_t ticks);
};
