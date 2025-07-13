#pragma once

#include "gamedata.h"
#include <entt/entity/fwd.hpp>
#include <canyon/utils/vector.h>

struct ComponentPickup {
    std::string m_name;
};

class SystemPickup {
public:
    static entt::entity CreatePickup(entt::registry& registry, canyon::FloatVec2 const& position,
                                     std::string const& name, Gamedata const& gamedata);
    static void Update(entt::registry& registry, uint32_t ticks, Gamedata const& gamedata);
};
