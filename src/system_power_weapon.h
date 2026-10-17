#pragma once

#include "gamedata.h"
#include "gamedata_projectile.h"
#include <cstdint>
#include <moth/ecs/world.h>
class GameWorld;
struct PowerWeaponData;

struct ComponentPowerWeapon {
    bool m_activated = false; // the button was pressed this frame
    float m_segmentPower = 0;
    int32_t m_missilesPerSegment = 0;
    float m_damage = 0;
    ProjectileData m_projectile;
    float m_muzzleSpeed = 0;
};

class SystemPowerWeapon {
public:
    static void InitWeapon(moth::ecs::World& registry, moth::ecs::Entity entity, PowerWeaponData const& data);
    static void Update(GameWorld& world, uint32_t ticks);
};
