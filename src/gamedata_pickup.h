#pragma once

#include "gamedata_database.h"
#include "gamedata_sprite.h"
#include <nlohmann/json_fwd.hpp>
#include <string>

enum class PickupType {
    Weapon,
    Passive,
};

struct PickupData {
    PickupType type;
    std::string name;
    float value;
    SpriteData sprite;

    static PickupData Deserialize(nlohmann::json const& json, SerializeContext const& context);
};
