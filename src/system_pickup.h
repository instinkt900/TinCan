#pragma once

#include "gamedata_sprite.h"
#include <entt/entity/fwd.hpp>

struct ComponentPickup {
    SpriteData m_sprite;
    std::string m_dropName;
};

class SystemPickup {
    public:
        static entt::entity CreatePickup(entt::registry& registry, moth_ui::FloatVec2 const& position, std::string const& name, Gamedata const& gamedata);
        static void Update(entt::registry& registry, uint32_t ticks);
};
