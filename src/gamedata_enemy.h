#pragma once

#include <canyon/graphics/color.h>
#include <nlohmann/detail/macro_scope.hpp>
#include "gamedata_sprite.h"
#include "gamedata_weapon.h"
#include "utils_serialization.h"
#include "dataref.h"

enum class EnergyColor {
    Blue,
    Red,
};
MAGIC_SERIALIZE_ENUM(EnergyColor);

inline canyon::graphics::Color GetEnergyColor(EnergyColor const& color) {
    return color == EnergyColor::Blue ? canyon::graphics::BasicColors::Blue
                                       : canyon::graphics::BasicColors::Red;
}

struct EnemyData {
    static constexpr GameDataCategory Category = GameDataCategory::Enemies;

    DataRef<SpriteData> sprite;
    EnergyColor color = EnergyColor::Blue;
    float health = 0;
    float radius = 0;
    DataRef<WeaponData> weapon;
};

inline void from_json(nlohmann::json const& json, EnemyData& data) {
    DATA_REQUIRED(json, data, sprite);
    DATA_REQUIRED(json, data, color);
    DATA_REQUIRED(json, data, health);
    DATA_REQUIRED(json, data, radius);
    DATA_REQUIRED(json, data, weapon);
}

