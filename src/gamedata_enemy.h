#pragma once

#include "dataref.h"
#include "gamedata.h"
#include "gamedata_behaviour.h"
#include "gamedata_entity.h"
#include "gamedata_pickup.h"
#include "gamedata_sprite.h"
#include "gamedata_weapon.h"
#include <optional>
#include <string>
#include <vector>

struct EnemySection {
    DataRef<SpriteData> sprite;
    moth::gfx::IntVec2 offset;
    float radius = 1.0f;
    float health = 1.0f;
    DataRef<WeaponData> weapon;
    DataRef<SpriteData> death_sprite;
    std::optional<ContactData> contact;
};

inline void from_json(nlohmann::json const& json, EnemySection& data) {
    DATA_REQUIRED(json, data, sprite);
    DATA_OPTIONAL(json, data, offset);
    DATA_OPTIONAL(json, data, radius);
    DATA_OPTIONAL(json, data, health);
    DATA_OPTIONAL(json, data, weapon);
    DATA_OPTIONAL(json, data, death_sprite);
    DATA_OPTIONAL(json, data, contact);
}

// Enemies spawned where an enemy is killed. The fields are those of a map's EnemySpawn, apart from where it
// spawns: the pieces are placed around the enemy that died. The enemy is named rather than a DataRef,
// because a DataRef holds its data by value and an enemy cannot hold an enemy.
struct EnemyDeathSpawn {
    std::string enemy_type;
    DataRef<BehaviourData> behaviour;
    int count = 1;
    std::string formation = "line";
    float spacing = 32.0f;
    float scale_x = 1.0f;
    float scale_y = 1.0f;
    // unset leaves the behaviour at the speed it was authored with
    std::optional<float> speed;
    float behaviour_rotation = 0.0f;
    float behaviour_rotation_step = 0.0f;
    DataRef<PickupData> drop;
};

inline void from_json(nlohmann::json const& json, EnemyDeathSpawn& data) {
    DATA_REQUIRED(json, data, enemy_type);
    DATA_OPTIONAL(json, data, behaviour);
    DATA_OPTIONAL(json, data, count);
    DATA_OPTIONAL(json, data, formation);
    DATA_OPTIONAL(json, data, spacing);
    DATA_OPTIONAL(json, data, scale_x);
    DATA_OPTIONAL(json, data, scale_y);
    DATA_OPTIONAL(json, data, speed);
    DATA_OPTIONAL(json, data, behaviour_rotation);
    DATA_OPTIONAL(json, data, behaviour_rotation_step);
    DATA_OPTIONAL(json, data, drop);
}

struct EnemyData {
    static constexpr GameDataCategory Category = GameDataCategory::Enemies;

    Affinity affinity = Affinity::None;
    DataRef<SpriteData> sprite;
    // Multiplies the spin rate its sprite was authored with, so this tweaks how fast a spinning enemy
    // turns without being able to start one spinning: a sprite with no spin stays still at any scale.
    // A negative scale turns the other way.
    float spin_scale = 1.0f;
    float radius = 0.0f;
    float health = 0.0f;
    DataRef<WeaponData> weapon;
    DataRef<SpriteData> death_sprite;
    std::optional<ContactData> contact;
    std::map<std::string, EnemySection> sections;
    // Milliseconds the enemy may live before it counts as escaped and is removed, whatever it is doing.
    uint32_t lifetime = 15000;
    // spawned where the enemy is killed, and not when it escapes
    std::vector<EnemyDeathSpawn> death_spawns;
    // points for killing it, by any means
    int32_t score = 0;
    // a large enemy's death has a bigger explosion, a shake and a hit-stop
    bool large = false;
};

inline void from_json(nlohmann::json const& json, EnemyData& data) {
    DATA_OPTIONAL(json, data, affinity);
    DATA_OPTIONAL(json, data, sprite);
    DATA_OPTIONAL(json, data, spin_scale);
    DATA_OPTIONAL(json, data, radius);
    DATA_OPTIONAL(json, data, health);
    DATA_OPTIONAL(json, data, weapon);
    DATA_OPTIONAL(json, data, death_sprite);
    DATA_OPTIONAL(json, data, contact);
    DATA_OPTIONAL(json, data, sections);
    DATA_OPTIONAL(json, data, lifetime);
    DATA_OPTIONAL(json, data, death_spawns);
    DATA_OPTIONAL(json, data, score);
    DATA_OPTIONAL(json, data, large);
}

