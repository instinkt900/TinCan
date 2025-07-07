#pragma once

#include "gamedata_sprite.h"
#include <string>

class Gamedata;

struct ProjectileData {
    std::string name;
    SpriteData white_sprite;
    SpriteData black_sprite;
    float radius;
    float damage;
    float speed;

    bool Load(nlohmann::json json, Gamedata const& gamedata, canyon::graphics::SurfaceContext& surfaceContext);
};
