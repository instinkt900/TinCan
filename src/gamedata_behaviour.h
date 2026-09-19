#pragma once

#include "gamedata.h"
#include "utils_paths.h"
#include "utils_serialization.h"

enum class BehaviourType {
    Undefined,
    Path,
    Target,
    Hold,
};
MAGIC_SERIALIZE_ENUM(BehaviourType);

enum class TargetType {
    Undefined,
    Point,
    Player,
};
MAGIC_SERIALIZE_ENUM(TargetType);

// PathInterpolation belongs to the sampler, which is what gives it meaning
MAGIC_SERIALIZE_ENUM(PathInterpolation);

// What the coordinates of a behaviour are measured against. A path is usually authored as a shape relative
// to wherever the enemy comes in, which is what SpawnRelative means.
enum class BehaviourSpace {
    SpawnRelative,
    World,
};
MAGIC_SERIALIZE_ENUM(BehaviourSpace);

struct BehaviourStateData {
    BehaviourType type{ BehaviourType::Undefined };
    std::string label;
    // 0 means the state runs until it completes on its own, which for a path is when it reaches its end
    uint32_t duration{ 0 };
    TargetType target{ TargetType::Undefined };
    moth::gfx::FloatVec2 point{ 0, 0 };
    std::vector<moth::gfx::FloatVec2> waypoints;
    PathInterpolation interpolation{ PathInterpolation::Linear };
    float speed{ 0 };
};

struct BehaviourData {
    static constexpr GameDataCategory Category = GameDataCategory::Behaviours;

    BehaviourSpace space{ BehaviourSpace::SpawnRelative };
    std::vector<BehaviourStateData> states;
};

inline void from_json(nlohmann::json const& json, BehaviourStateData& data) {
    DATA_REQUIRED(json, data, type);
    DATA_OPTIONAL(json, data, label);
    DATA_OPTIONAL(json, data, duration);

    if (data.type == BehaviourType::Path) {
        DATA_REQUIRED(json, data, waypoints);
        DATA_OPTIONAL(json, data, interpolation);
        DATA_REQUIRED(json, data, speed);
    } else if (data.type == BehaviourType::Target) {
        DATA_REQUIRED(json, data, target);
        DATA_OPTIONAL(json, data, point);
        DATA_OPTIONAL(json, data, speed);
    }
}

inline void from_json(nlohmann::json const& json, BehaviourData& data) {
    DATA_OPTIONAL(json, data, space);
    DATA_REQUIRED(json, data, states);
}
