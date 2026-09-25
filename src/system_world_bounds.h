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

// An enemy becomes active once its position is on screen. From then on, leaving the screen and a small
// border around it counts as an escape. Before that it waits off screen and only its lifetime removes it.
struct ComponentScreenExit {
    bool m_active = false;
};

class SystemWorldBounds {
    public:
        static void Update(GameWorld& world, uint32_t ticks);
};

