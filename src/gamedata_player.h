#pragma once

#include "dataref.h"
#include "gamedata.h"
#include "gamedata_entity.h"
#include "gamedata_movement.h"
#include "gamedata_sprite.h"
#include "gamedata_weapon.h"
#include <optional>

inline constexpr char kDefaultPlayerName[] = "default";

// The power weapon. Power is stored in segments, and a press spends every full one on a swarm of homing missiles.
struct PowerWeaponData {
    float max_power = 100.0f;
    float segment_power = 10.0f;
    float absorb_power = 1.0f; // power for each enemy bullet the shield absorbs
    int32_t missiles_per_segment = 4;
    float missile_damage = 50.0f;
    DataRef<ProjectileData> projectile;
    float muzzle_speed = 200.0f;
};

inline void from_json(nlohmann::json const& json, PowerWeaponData& data) {
    DATA_OPTIONAL(json, data, max_power);
    DATA_OPTIONAL(json, data, segment_power);
    DATA_OPTIONAL(json, data, absorb_power);
    DATA_OPTIONAL(json, data, missiles_per_segment);
    DATA_OPTIONAL(json, data, missile_damage);
    DATA_REQUIRED(json, data, projectile);
    DATA_OPTIONAL(json, data, muzzle_speed);
}

struct PlayerData {
    static constexpr GameDataCategory Category = GameDataCategory::Players;

    Affinity affinity = Affinity::Light;
    DataRef<SpriteData> sprite;
    float radius = 5.0f;
    MovementData movement;
    std::string weapon; // the progression the player starts with, at its first level
    float max_level_power = 20.0f; // power given by a weapon pickup that has no level left to give
    DataRef<SpriteData> shield_sprite;
    float shield_radius = 45.0f;
    DataRef<SpriteData> death_sprite;
    float death_sprite_scale = 2.0f; // the ship's explosion is bigger than its sprite
    std::optional<ContactData> contact;
    int32_t lives = 3;
    int32_t max_lives = 5;
    uint32_t respawn_delay = 1500; // milliseconds from the ship being destroyed to the next one flying in
    uint32_t fly_in_time = 1000;   // milliseconds the next ship takes to fly in from below the screen
    uint32_t safe_time = 3000;     // milliseconds the next ship is safe once it arrives
    uint32_t drop_lifetime = 8000; // milliseconds a weapon dropped on death stays before it is gone
    int32_t extra_life_score = 100000; // points for each extra life
    std::optional<PowerWeaponData> power_weapon;
    DataRef<SpriteData> muzzle_flash_sprite; // shown at each barrel as a player weapon fires
    DataRef<SpriteData> hit_spark_sprite;    // shown where a player shot hits an enemy
};

inline void from_json(nlohmann::json const& json, PlayerData& data) {
    DATA_OPTIONAL(json, data, affinity);
    DATA_REQUIRED(json, data, sprite);
    DATA_OPTIONAL(json, data, radius);
    DATA_OPTIONAL(json, data, movement);
    DATA_REQUIRED(json, data, weapon);
    DATA_OPTIONAL(json, data, max_level_power);
    DATA_OPTIONAL(json, data, shield_sprite);
    DATA_OPTIONAL(json, data, shield_radius);
    DATA_OPTIONAL(json, data, death_sprite);
    DATA_OPTIONAL(json, data, death_sprite_scale);
    DATA_OPTIONAL(json, data, contact);
    DATA_OPTIONAL(json, data, lives);
    DATA_OPTIONAL(json, data, max_lives);
    DATA_OPTIONAL(json, data, respawn_delay);
    DATA_OPTIONAL(json, data, fly_in_time);
    DATA_OPTIONAL(json, data, safe_time);
    DATA_OPTIONAL(json, data, drop_lifetime);
    DATA_OPTIONAL(json, data, extra_life_score);
    DATA_OPTIONAL(json, data, power_weapon);
    DATA_OPTIONAL(json, data, muzzle_flash_sprite);
    DATA_OPTIONAL(json, data, hit_spark_sprite);
}
