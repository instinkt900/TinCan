#include "gamedata_weapon.h"
#include <nlohmann/json.hpp>
#include <moth_ui/utils/vector_serialization.h>

bool WeaponData::Load(nlohmann::json const& json, Gamedata const& gamedata, canyon::graphics::SurfaceContext& surfaceContext) {
    json["name"].get_to(name);
    json["cooldown"].get_to(cooldown);
    json["player_tracking"].get_to(player_tracking);
    json["projectile_name"].get_to(projectile_name);
    for (auto const& barrelJson: json["barrels"]) {
        auto& barrel = barrels.emplace_back();
        barrelJson["offset"].get_to(barrel.offset);
        barrelJson["group"].get_to(barrel.group);
    }

    return true;
}
