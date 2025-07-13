#pragma once

#include <entt/entity/fwd.hpp>
#include <canyon/utils/rect.h>

class GameWorld;

enum class BoundsBehaviour {
    Unknown = -1,
    Limit,  // Limit the movement of the entity to the bounds.
    Kill,   // Kill the entity if it goes outside the bounds.
};

struct ComponentBounds {
    BoundsBehaviour m_behaviour = BoundsBehaviour::Unknown;
    canyon::FloatRect m_bounds;
};

class SystemWorldBounds {
    public:
        static void Update(GameWorld& world, uint32_t ticks);
};

