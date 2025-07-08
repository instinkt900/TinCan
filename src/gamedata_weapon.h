#pragma once

#include "gamedata_database.h"
#include <moth_ui/utils/vector.h>

struct BarrelData {
    moth_ui::IntVec2 offset;
    int32_t group = 0;
};

struct WeaponData {
    int32_t cooldown;
    bool player_tracking;
    std::string projectile_name;
    std::vector<BarrelData> barrels;

    static WeaponData Deserialize(nlohmann::json const& json, SerializeContext const& gamedata);
};

