#pragma once

#include "dataref.h"
#include "gamedata.h"
#include "gamedata_entity.h"
#include "gamedata_sprite.h"
#include "gamedata_weapon.h"

struct EnemySection {
    DataRef<SpriteData> sprite;
    canyon::IntVec2 offset;
    float radius = 1.0f;
    float health = 1.0f;
    DataRef<WeaponData> weapon;
};

inline void from_json(nlohmann::json const& json, EnemySection& data) {
    DATA_REQUIRED(json, data, sprite);
    DATA_OPTIONAL(json, data, offset);
    DATA_OPTIONAL(json, data, radius);
    DATA_OPTIONAL(json, data, health);
    DATA_OPTIONAL(json, data, weapon);
}

struct EnemyData {
    static constexpr GameDataCategory Category = GameDataCategory::Enemies;

    Affinity affinity = Affinity::None;
    DataRef<SpriteData> sprite;
    float radius = 0.0f;
    float health = 0.0f;
    DataRef<WeaponData> weapon;
    std::map<std::string, EnemySection> sections;
};

inline void from_json(nlohmann::json const& json, EnemyData& data) {
    DATA_OPTIONAL(json, data, affinity);
    DATA_OPTIONAL(json, data, sprite);
    DATA_OPTIONAL(json, data, radius);
    DATA_OPTIONAL(json, data, health);
    DATA_OPTIONAL(json, data, weapon);
    DATA_OPTIONAL(json, data, sections);
}

