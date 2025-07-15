#pragma once

#include "component_entity.h"
#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <canyon/utils/vector.h>

class GameWorld;

struct ComponentProjectile {
    entt::entity m_owner = entt::null;
    EnergyColor m_color = EnergyColor::Blue;
    float m_damage = 1;
};

class SystemProjectile {
public:
    static void Update(GameWorld& world, uint32_t ticks);
};
