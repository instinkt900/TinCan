#include "gamedata_enemy.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include "gamedata.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EnemyData, health, radius, lifetime, weapon_name);

EnemyData EnemyData::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
    EnemyData data = json.get<EnemyData>();

    auto const* spriteDatabase = context.gamedata.GetSpriteDatabase();
    if (spriteDatabase == nullptr) {
        throw std::runtime_error("Unable to read sprite database.");
    }

    auto const* spriteData = spriteDatabase->Get(json["sprite_name"]);
    if (spriteData != nullptr) {
        data.sprite = *spriteData;
    }

    return data;
}
