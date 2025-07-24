#pragma once

#include "gamedata_pickup.h"
#include "gamedata_projectile.h"
#include <canyon/utils/vector.h>

struct BarrelData {
    canyon::FloatVec2 offset;
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
    std::optional<DataRef<PickupData>> pickup;
    std::vector<BarrelData> barrels;
};

inline void from_json(nlohmann::json const& json, WeaponData& data) {
    DATA_REQUIRED(json, data, damage);
    DATA_REQUIRED(json, data, cooldown);
    DATA_REQUIRED(json, data, burst);
    DATA_REQUIRED(json, data, burst_delay);
    DATA_REQUIRED(json, data, player_tracking);
    DATA_REQUIRED(json, data, projectile);
    DATA_OPTIONAL(json, data, pickup);
    DATA_REQUIRED(json, data, barrels);
}

