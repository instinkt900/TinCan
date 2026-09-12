#pragma once

#include <moth/ecs/world.h>
#include <moth/graphics/utils/rect.h>

class GameWorld;

enum class BoundsBehaviour {
    Unknown = -1,
    Limit,  // Limit the movement of the entity to the bounds.
    Kill,   // Kill the entity if it goes outside the bounds.
};

struct ComponentBounds {
    BoundsBehaviour m_behaviour = BoundsBehaviour::Unknown;
    moth::gfx::FloatRect m_bounds;
};

class SystemWorldBounds {
    public:
        static void Update(GameWorld& world, uint32_t ticks);
};

