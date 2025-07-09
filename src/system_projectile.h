#pragma once

#include "gamedata.h"
#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <moth_ui/utils/vector.h>

enum class Team {
    NONE = 0,
    PLAYER = 1,
    ENEMY = 2,
};

enum class EnergyColor {
    WHITE,
    BLACK,
};

struct ComponentProjectile {
    entt::entity m_owner = entt::null;
    EnergyColor m_color = EnergyColor::WHITE;
    float m_damage = 1;
};

class SystemProjectile {
public:
    static void Update(entt::registry& registry, uint32_t ticks, Gamedata const& gamedata);
};
