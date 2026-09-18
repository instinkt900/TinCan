#pragma once

#include "gamedata.h"
#include "gamedata_pickup.h"
#include <moth/ecs/world.h>
#include <moth/graphics/utils/vector.h>

class GameWorld;

struct ComponentPickup {
    PickupData m_data;
};

class SystemPickup {
public:
    static moth::ecs::Entity CreatePickup(moth::ecs::World& registry, moth::gfx::FloatVec2 const& position,
                                     PickupData const& pickupData, GameData const& gamedata);
    static void Update(GameWorld& world, uint32_t ticks);
};
