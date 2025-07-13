#pragma once

#include "component_entity.h"
#include "gamedata.h"
#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <canyon/utils/vector.h>

struct ComponentProjectile {
    entt::entity m_owner = entt::null;
    EnergyColor m_color = EnergyColor::WHITE;
    float m_damage = 1;
};

class SystemProjectile {
public:
    static void Update(entt::registry& registry, uint32_t ticks, Gamedata const& gamedata);
};
