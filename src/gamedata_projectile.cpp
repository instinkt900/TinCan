#include "gamedata_projectile.h"
#include <nlohmann/json.hpp>
#include "gamedata.h"

bool ProjectileData::Load(nlohmann::json json, Gamedata const& gamedata,
                          canyon::graphics::SurfaceContext& surfaceContext) {
    auto const* spriteDatabase = gamedata.GetSpriteDatabase();
    if (spriteDatabase == nullptr) {
        return false;
    }

    auto const* whiteSpriteData = spriteDatabase->Get(json["white_sprite"]);
    if (whiteSpriteData != nullptr) {
        white_sprite = *whiteSpriteData;
    }
    auto const* blackSpriteData = spriteDatabase->Get(json["black_sprite"]);
    if (blackSpriteData != nullptr) {
        black_sprite = *blackSpriteData;
    }

    json["name"].get_to(name);
    json["radius"].get_to(radius);
    json["damage"].get_to(damage);
    json["speed"].get_to(speed);

    return true;
}
