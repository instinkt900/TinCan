#pragma once

#include "dataref.h"
#include "gamedata.h"
#include "utils_serialization.h"
#include "gamedata_sprite.h"

enum class PickupType {
    Weapon,
    Passive,
};
MAGIC_SERIALIZE_ENUM(PickupType)

struct PickupData {
    static constexpr GameDataCategory Category = GameDataCategory::Pickups;
    
    DataRef<SpriteData> sprite;
    PickupType type;
    std::string name;
    std::optional<float> value;
};

inline void from_json(nlohmann::json const& json, PickupData& data) {
    DATA_REQUIRED(json, data, sprite);
    DATA_REQUIRED(json, data, type);
    DATA_REQUIRED(json, data, name);
    DATA_OPTIONAL(json, data, value);
}
