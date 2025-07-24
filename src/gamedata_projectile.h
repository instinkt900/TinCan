#pragma once

#include "dataref.h"
#include "gamedata_sprite.h"
#include <nlohmann/detail/macro_scope.hpp>
#include <optional>
#include "utils_serialization.h"

enum class Targeting {
    None,
    Nearest,
    Random,
};
MAGIC_SERIALIZE_ENUM(Targeting);

struct HomingDetails {
    Targeting targeting = Targeting::None;
    float turn_speed = 0;
    float max_thrust = 0;
    float max_speed = 0;
    float min_drag = 0;
    float max_drag = 0;
};

inline void from_json(nlohmann::json const& json, HomingDetails& data) {
    DATA_REQUIRED(json, data, targeting);
    DATA_REQUIRED(json, data, turn_speed);
    DATA_REQUIRED(json, data, max_thrust);
    DATA_REQUIRED(json, data, max_speed);
    DATA_REQUIRED(json, data, min_drag);
    DATA_REQUIRED(json, data, max_drag);
}

struct ProjectileData {
    static constexpr GameDataCategory Category = GameDataCategory::Projectiles;

    DataRef<SpriteData> white_sprite;
    DataRef<SpriteData> black_sprite;
    float radius = 0;
    float speed = 0;
    std::optional<HomingDetails> homing_details;
};

inline void from_json(nlohmann::json const& json, ProjectileData& data) {
    DATA_REQUIRED(json, data, white_sprite);
    DATA_REQUIRED(json, data, black_sprite);
    DATA_REQUIRED(json, data, radius);
    DATA_REQUIRED(json, data, speed);
    DATA_OPTIONAL(json, data, homing_details);
}
