#pragma once

#include "gamedata.h"
#include <entt/entity/fwd.hpp>
#include <canyon/graphics/surface_context.h>
#include <canyon/utils/vector.h>

class GameWorld;

struct Barrel {
    canyon::FloatVec2 m_offset;
    float m_angle = 0;
};

using BarrelGroup = std::vector<Barrel>;

struct ComponentWeapon {
    bool m_active = false;
    std::optional<std::string> m_pickupName; // if this weapon is dropped, this pickup is dropped
    float m_damage = 0;
    int32_t m_cooldown = 0;
    int32_t m_maxCooldown = 1000;
    int32_t m_burst = 0;
    int32_t m_maxBurst = 1;
    int32_t m_burstCooldown = 0;
    int32_t m_maxBurstCooldown = 0;
    bool m_playerTracking = false;
    std::string m_projectileName;
    int32_t m_barrelGroupIndex = 0;
    std::vector<int32_t> m_barrelGroupIds;
    std::map<int32_t, BarrelGroup> m_barrelGroups;
};

class SystemWeapon {
public:
    static ComponentWeapon* InitWeapon(entt::registry& registry, entt::entity entity, std::string const& name, GameData const& gamedata);
    static void Update(GameWorld& world, uint32_t ticks);
};
