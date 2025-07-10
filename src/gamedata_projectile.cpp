#include "gamedata_projectile.h"
#include <nlohmann/json.hpp>
#include "gamedata.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProjectileData, radius, damage, speed);

ProjectileData ProjectileData::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
    ProjectileData data = json.get<ProjectileData>();

    auto const* whiteSpriteData = context.gamedata.GetSpriteDatabase().Get(json["white_sprite"]);
    if (whiteSpriteData != nullptr) {
        data.white_sprite = *whiteSpriteData;
    }
    auto const* blackSpriteData = context.gamedata.GetSpriteDatabase().Get(json["black_sprite"]);
    if (blackSpriteData != nullptr) {
        data.black_sprite = *blackSpriteData;
    }

    return data;
}
