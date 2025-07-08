#pragma once

#include "gamedata_sprite.h"

struct ProjectileData {
    SpriteData white_sprite;
    SpriteData black_sprite;
    float radius = 0;
    float damage = 0;
    float speed = 0;

    static ProjectileData Deserialize(nlohmann::json const& json, SerializeContext const& context);
};
