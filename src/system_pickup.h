#pragma once

#include "gamedata.h"
#include "gamedata_pickup.h"
#include <moth/ecs/world.h>
#include <moth/graphics/utils/vector.h>

class GameWorld;

struct ComponentPickup {
    PickupData m_data;
    int32_t m_levels = 1; // the weapon levels it gives; a weapon dropped on death gives all the levels it had
};

class SystemPickup {
public:
    static moth::ecs::Entity CreatePickup(moth::ecs::World& registry, moth::gfx::FloatVec2 const& position,
                                     PickupData const& pickupData, GameData const& gamedata);
    // drops every weapon the destroyed ship carries, apart from the starting one, as a pickup of its own that
    // gives back the levels it had
    static void DropWeapons(moth::ecs::World& registry, moth::ecs::Entity player, GameData const& gamedata);
    static void Update(GameWorld& world, uint32_t ticks);
};
