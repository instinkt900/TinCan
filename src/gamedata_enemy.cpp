#include "gamedata_enemy.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include "gamedata.h"

bool EnemyData::Load(nlohmann::json const& json, Gamedata const& gamedata, canyon::graphics::SurfaceContext& surfaceContext) {
    auto const* spriteDatabase = gamedata.GetSpriteDatabase();
    if (spriteDatabase == nullptr) {
        return false;
    }

    auto const* spriteData = spriteDatabase->Get(json["sprite_name"]);
    if (spriteData != nullptr) {
        sprite = *spriteData;
    }

    json["name"].get_to(name);
    json["health"].get_to(health);
    json["radius"].get_to(radius);
    json["lifetime"].get_to(lifetime);
    json["weapon_name"].get_to(weapon_name);
    return true;
}
