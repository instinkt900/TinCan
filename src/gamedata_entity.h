#pragma once

#include "dataref.h"
#include "gamedata_sprite.h"
#include "gamedata_weapon.h"

enum class Team {
    None = -1,
    Player,
    Enemy,
};
MAGIC_SERIALIZE_ENUM(Team);

enum class Affinity {
    None = -1,
    Light,
    Dark
};
MAGIC_SERIALIZE_ENUM(Affinity);

struct EntitySection {
    DataRef<SpriteData> sprite;
    canyon::IntVec2 offset;
    float radius = 1.0f;
    float health = 1.0f;
    DataRef<WeaponData> weapon;
};

inline void from_json(nlohmann::json const& json, EntitySection& data) {
    DATA_REQUIRED(json, data, sprite);
    DATA_OPTIONAL(json, data, offset);
    DATA_OPTIONAL(json, data, radius);
    DATA_OPTIONAL(json, data, health);
    DATA_OPTIONAL(json, data, weapon);
}

