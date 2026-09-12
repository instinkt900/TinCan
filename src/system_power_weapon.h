#pragma once

#include "gamedata.h"
#include <cstdint>
#include <moth/ecs/world.h>
#include <string>
class GameWorld;

struct ComponentPowerWeapon {
    bool m_activated;
    float m_damage;
    std::string m_projectileName;
    float m_powerPerProjectile;
};

class SystemPowerWeapon {
public:
    static void InitWeapon(moth::ecs::World& registry, moth::ecs::Entity entity, std::string const& name, GameData const& gamedata);
    static void Update(GameWorld& world, uint32_t ticks);
};
