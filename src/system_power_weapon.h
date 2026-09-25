#pragma once

#include "gamedata.h"
#include <cstdint>
#include <moth/ecs/world.h>
#include <string>
class GameWorld;

struct ComponentPowerWeapon {
    bool m_activated = false;
    float m_damage = 0;
    std::string m_projectileName;
    float m_powerPerProjectile = 0;
    float m_muzzleSpeed = 0;
};

class SystemPowerWeapon {
public:
    static void InitWeapon(moth::ecs::World& registry, moth::ecs::Entity entity, std::string const& name, GameData const& gamedata);
    static void Update(GameWorld& world, uint32_t ticks);
};
