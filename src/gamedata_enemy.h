#pragma once

#include "gamedata_sprite.h"
#include <string>
#include <nlohmann/json.hpp>

struct EnemyData {
    SpriteData sprite;
    float health;
    float radius;
    uint32_t lifetime;
    std::string weapon_name;

    static EnemyData Deserialize(nlohmann::json const& json, SerializeContext const& context);
};

