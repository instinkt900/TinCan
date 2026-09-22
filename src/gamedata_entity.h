#pragma once

#include "utils_serialization.h"
#include <cstdint>

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


// What an entity does when its body touches another entity's body. An entity takes part in contact
// collisions only if its data carries one of these, so leaving the block out is how a thing is made
// to pass straight through everything.
struct ContactData {
    // dealt to whatever it touches
    float damage = 0.0f;
    // dealt to itself on contact, so a thing that shatters on impact says so here rather than needing
    // its own flag
    float self_damage = 0.0f;
    // milliseconds to wait after a contact before it can touch anything again. Without one, two
    // overlapping bodies collide again every frame for as long as they overlap.
    int32_t cooldown = 0;
};

inline void from_json(nlohmann::json const& json, ContactData& data) {
    DATA_OPTIONAL(json, data, damage);
    DATA_OPTIONAL(json, data, self_damage);
    DATA_OPTIONAL(json, data, cooldown);
}
