#pragma once

#include <entt/entity/fwd.hpp>

class GameWorld;

class SystemPlayerVisuals {
public:
    static void Update(GameWorld& world, uint32_t ticks);
};
