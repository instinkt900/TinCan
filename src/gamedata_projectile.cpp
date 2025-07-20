#include "gamedata_projectile.h"
#include <nlohmann/json.hpp>
#include "gamedata.h"
#include <canyon/utils/math_utils.h>
#include "serialization_utils.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProjectileData, radius, speed);

ProjectileData ProjectileData::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
    ProjectileData data = json.get<ProjectileData>();

    opt_get(json, data, targeting);
    opt_get(json, data, turn_speed);
    opt_get(json, data, max_thrust);
    opt_get(json, data, max_speed);
    opt_get(json, data, min_drag);
    opt_get(json, data, max_drag);

    auto const* whiteSpriteData = context.gamedata.GetSpriteDatabase().Get(json["white_sprite"]);
    if (whiteSpriteData != nullptr) {
        data.white_sprite = *whiteSpriteData;
    }
    auto const* blackSpriteData = context.gamedata.GetSpriteDatabase().Get(json["black_sprite"]);
    if (blackSpriteData != nullptr) {
        data.black_sprite = *blackSpriteData;
    }

    if (data.turn_speed.has_value()) {
        data.turn_speed = canyon::Radians(data.turn_speed.value());
    }

    return data;
}
