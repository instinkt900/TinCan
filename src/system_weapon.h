#pragma once

#include "gamedata.h"
#include "gamedata_projectile.h"
#include <moth/ecs/world.h>
#include <moth/graphics/graphics/surface_context.h>
#include <moth/graphics/utils/vector.h>
#include <string>
#include <vector>

class GameWorld;

struct Barrel {
    moth::gfx::FloatVec2 m_offset;
    float m_angle = 0;
};

using BarrelGroup = std::vector<Barrel>;

struct Weapon {
    bool m_active = false;
    std::string m_name; // the weapon's name in the database, used to spot one already carried
    std::string m_progression; // the progression the weapon is a level of; empty for a weapon with no levels
    int32_t m_level = 0;       // index of the weapon's level in its progression
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
    float m_muzzleSpeed = 0;
    int32_t m_barrelGroupIndex = 0;
    std::vector<int32_t> m_barrelGroupIds;
    std::map<int32_t, BarrelGroup> m_barrelGroups;

    Weapon(std::string name, WeaponData const& weapon);
};

// an entity fires every weapon it carries at once
struct ComponentWeapon {
    std::vector<Weapon> m_weapons;
};

class SystemWeapon {
public:
    // replaces whatever the entity carries with this single weapon
    static Weapon* InitWeapon(moth::ecs::World& registry, moth::ecs::Entity entity, std::string const& name,
                              WeaponData const& weaponData, GameData const& gamedata, bool active = false);
    // adds a weapon alongside the ones the entity already carries. a weapon it already has is left
    // alone rather than added twice
    static Weapon* AddWeapon(moth::ecs::World& registry, moth::ecs::Entity entity, std::string const& name,
                             WeaponData const& weaponData, GameData const& gamedata, bool active = false);
    // gives the entity levels of a progression's weapon: one it does not carry is added at the first level and
    // takes the rest from there, and one it carries is raised by that many. the level stops at the last, and
    // the return is how many levels did not fit
    static int32_t AddWeaponLevels(moth::ecs::World& registry, moth::ecs::Entity entity,
                                   std::string const& progressionName, int32_t levels, GameData const& gamedata);
    static void Update(GameWorld& world, uint32_t ticks);
};
