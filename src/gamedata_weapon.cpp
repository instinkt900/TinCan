#include "gamedata_weapon.h"
#include <nlohmann/json.hpp>
#include <moth_ui/utils/vector_serialization.h>

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BarrelData, offset, group);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WeaponData, cooldown, player_tracking, projectile_name, barrels);

WeaponData WeaponData::Deserialize(nlohmann::json const& json, SerializeContext const& gamedata) {
    return json.get<WeaponData>();
}
