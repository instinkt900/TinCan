#pragma once

#include "component_entity.h"
#include "gamedata_projectile.h"
#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <canyon/utils/vector.h>

class GameWorld;

struct ComponentProjectile {
    entt::entity m_source = entt::null;
    EnergyColor m_color = EnergyColor::Blue;
    float m_damage = 1;
};

class SystemProjectile {
public:
    static entt::entity CreateProjectile(entt::registry& registry, ProjectileData const& data,
                                         entt::entity source, canyon::FloatVec2 const& position,
                                         canyon::FloatVec2 const& direction, float rotation, float damage);
    static void Update(GameWorld& world, uint32_t ticks);
};
