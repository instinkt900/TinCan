#pragma once

#include "gamedata_sprite.h"
#include <string>

class Gamedata;

struct EnemyData {
    std::string name;
    SpriteData sprite;
    float health;
    float radius;
    uint32_t lifetime;
    std::string weapon_name;

    bool Load(nlohmann::json const& json, Gamedata const& gamedata, canyon::graphics::SurfaceContext& surfaceContext);
};

