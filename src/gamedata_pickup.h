#pragma once

#include "dataref.h"
#include "gamedata_sprite.h"
#include <nlohmann/detail/macro_scope.hpp>
#include <string>
#include "utils_serialization.h"

enum class PickupType {
    Weapon,
    Passive,
};
MAGIC_SERIALIZE_ENUM(PickupType)

struct PickupData {
    static constexpr GameDataCategory Category = GameDataCategory::Pickups;
    
    PickupType type;
    std::string name;
    std::optional<float> value;
    DataRef<SpriteData> sprite;
};

inline void from_json(nlohmann::json const& json, PickupData& data) {
    DATA_REQUIRED(json, data, type);
    DATA_REQUIRED(json, data, type);
    DATA_OPTIONAL(json, data, value);
    DATA_REQUIRED(json, data, sprite);
}
