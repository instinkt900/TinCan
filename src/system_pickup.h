#pragma once

#include "gamedata.h"
#include <entt/entity/fwd.hpp>
#include <moth_graphics/utils/vector.h>

class GameWorld;

struct ComponentPickup {
    std::string m_name;
};

class SystemPickup {
public:
    static entt::entity CreatePickup(entt::registry& registry, moth_graphics::FloatVec2 const& position,
                                     PickupData const& pickupData, GameData const& gamedata);
    static void Update(GameWorld& world, uint32_t ticks);
};
