#pragma once

#include "dataref.h"
#include "gamedata.h"
#include "gamedata_entity.h"
#include "gamedata_movement.h"
#include "gamedata_sprite.h"
#include "gamedata_weapon.h"
#include <optional>

inline constexpr char kDefaultPlayerName[] = "default";

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
}
