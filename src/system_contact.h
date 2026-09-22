#pragma once

#include <moth/ecs/world.h>

class GameWorld;

// Body-to-body collisions between entities on opposing teams: the player flying into an asteroid
// rather than anything being shot.
//
// Contact is symmetric and opt-in from both sides. Only entities carrying a ComponentContact are
// considered, and on a touch each side applies its own damage to the other, so an entity with no
// contact data in its game data passes straight through everything.
class SystemContact {
public:
    static void Update(GameWorld& world, uint32_t ticks);
};
