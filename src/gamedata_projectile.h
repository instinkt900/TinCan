#pragma once

#include "gamedata_sprite.h"
#include "utils.h"

enum Targeting {
    None,
    Nearest,
    Random,
};

MAGIC_SERIALIZE_ENUM(Targeting);

struct ProjectileData {
    SpriteData white_sprite;
    SpriteData black_sprite;
    float radius = 0;
    float damage = 0;
    float speed = 0;

    std::optional<Targeting> targeting;
    std::optional<float> turn_speed = 0;
    std::optional<float> max_thrust = 0;
    std::optional<float> max_speed = 0;
    std::optional<float> min_drag = 0;
    std::optional<float> max_drag = 0;

    static ProjectileData Deserialize(nlohmann::json const& json, SerializeContext const& context);
};
