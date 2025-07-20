#pragma once

#include "gamedata_database.h"
#include <canyon/utils/vector.h>

struct BarrelData {
    canyon::FloatVec2 offset;
    int32_t group = 0;
    float angle = 0;
};

struct WeaponData {
    float damage = 0;
    int32_t cooldown;
    int32_t burst;
    int32_t burst_delay;
    bool player_tracking;
    std::string projectile_name;
    std::optional<std::string> pickup_name;
    std::vector<BarrelData> barrels;

    static WeaponData Deserialize(nlohmann::json const& json, SerializeContext const& gamedata);
};

