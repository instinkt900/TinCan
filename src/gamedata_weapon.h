#pragma once

#include "gamedata_projectile.h"
#include <moth/graphics/utils/vector.h>

struct BarrelData {
    moth::gfx::FloatVec2 offset;
    int32_t group = 0;
    float angle = 0;
};

inline void from_json(nlohmann::json const& json, BarrelData& data) {
    DATA_REQUIRED(json, data, offset);
    DATA_REQUIRED(json, data, group);
    DATA_REQUIRED(json, data, angle);
}

struct WeaponData {
    static constexpr GameDataCategory Category = GameDataCategory::Weapons;

    float damage = 0;
    int32_t cooldown;
    int32_t burst;
    int32_t burst_delay;
    bool player_tracking;
    DataRef<ProjectileData> projectile;
    // how fast a projectile leaves the barrel. any propulsion after that is the projectile's own
    float muzzle_speed = 0;
    std::string fire_state; // sprite animation state entered on each shot; empty for none
    std::vector<BarrelData> barrels;
};

inline void from_json(nlohmann::json const& json, WeaponData& data) {
    DATA_REQUIRED(json, data, damage);
    DATA_REQUIRED(json, data, cooldown);
    DATA_REQUIRED(json, data, burst);
    DATA_REQUIRED(json, data, burst_delay);
    DATA_REQUIRED(json, data, player_tracking);
    DATA_REQUIRED(json, data, projectile);
    DATA_REQUIRED(json, data, muzzle_speed);
    DATA_OPTIONAL(json, data, fire_state);
    DATA_REQUIRED(json, data, barrels);
}

