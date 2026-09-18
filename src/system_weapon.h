#pragma once

#include "gamedata.h"
#include "gamedata_projectile.h"
#include "gamedata_pickup.h"
#include <moth/ecs/world.h>
#include <moth/graphics/graphics/surface_context.h>
#include <moth/graphics/utils/vector.h>

class GameWorld;

struct Barrel {
    moth::gfx::FloatVec2 m_offset;
    float m_angle = 0;
};

using BarrelGroup = std::vector<Barrel>;

struct ComponentWeapon {
    bool m_active = false;
    DataRef<PickupData> m_pickup; // if this weapon is dropped, this pickup is dropped
    float m_damage = 0;
    int32_t m_cooldown = 0;
    int32_t m_maxCooldown = 1000;
    int32_t m_burst = 0;
    int32_t m_maxBurst = 1;
    int32_t m_burstCooldown = 0;
    int32_t m_maxBurstCooldown = 0;
    bool m_playerTracking = false;
    std::string m_fireState;
    ProjectileData m_projectile;
    int32_t m_barrelGroupIndex = 0;
    std::vector<int32_t> m_barrelGroupIds;
    std::map<int32_t, BarrelGroup> m_barrelGroups;

    ComponentWeapon(WeaponData const& weapon);
};

ComponentWeapon InitWeapon(WeaponData const& data);

class SystemWeapon {
public:
    static ComponentWeapon* InitWeapon(moth::ecs::World& registry, moth::ecs::Entity entity,
                                       WeaponData const& weaponData, GameData const& gamedata,
                                       bool active = false);
    static void Update(GameWorld& world, uint32_t ticks);
};
