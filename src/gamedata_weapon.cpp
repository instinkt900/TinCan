#include "gamedata_weapon.h"
#include <nlohmann/json.hpp>
#include <canyon/utils/math_utils.h>

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BarrelData, offset, group, angle);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WeaponData, damage, cooldown, burst, burst_delay, player_tracking,
                                   projectile_name, barrels);

WeaponData WeaponData::Deserialize(nlohmann::json const& json, SerializeContext const& gamedata) {
    auto weaponData = json.get<WeaponData>();

    if (json.contains("pickup_name")) {
        weaponData.pickup_name = json["pickup_name"];
    }

    for (auto& barrel : weaponData.barrels) {
        barrel.angle = canyon::Radians(barrel.angle);
    }
    return weaponData;
}
