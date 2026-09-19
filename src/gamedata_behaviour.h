#pragma once

#include "gamedata.h"
#include "utils_serialization.h"

enum class BehaviourType {
    Undefined,
    Line,
    Target,
    Hold,
    Wave,
    Spline,
};
MAGIC_SERIALIZE_ENUM(BehaviourType);

enum class TargetType {
    Undefined,
    Point,
    Player,
};
MAGIC_SERIALIZE_ENUM(TargetType);

struct BehaviourStateData {
    BehaviourType type{ BehaviourType::Undefined };
    std::string label;
    uint32_t duration{ 0 };
    moth::gfx::FloatVec2 velocity{ 0, 0 };
    TargetType target{ TargetType::Undefined };
    moth::gfx::FloatVec2 point{ 0, 0 };
    std::vector<moth::gfx::FloatVec2> points;
    float speed{ 0 };
    float frequency{ 0 };
    float amplitude{ 0 };
};

struct BehaviourData {
    static constexpr GameDataCategory Category = GameDataCategory::Behaviours;

    std::vector<BehaviourStateData> states;
};

inline void from_json(nlohmann::json const& json, BehaviourStateData& data) {
    DATA_REQUIRED(json, data, type);
    DATA_OPTIONAL(json, data, label);
    DATA_OPTIONAL(json, data, duration);

    if (data.type == BehaviourType::Line) {
        DATA_REQUIRED(json, data, velocity);
    } else if (data.type == BehaviourType::Target) {
        DATA_REQUIRED(json, data, target);
        DATA_OPTIONAL(json, data, point);
        DATA_OPTIONAL(json, data, speed);
    } else if (data.type == BehaviourType::Wave) {
        DATA_REQUIRED(json, data, speed);
        DATA_REQUIRED(json, data, frequency);
        DATA_REQUIRED(json, data, amplitude);
    } else if (data.type == BehaviourType::Spline) {
        DATA_REQUIRED(json, data, speed);
        DATA_REQUIRED(json, data, points);
        // the endpoint padding a Catmull-Rom spline needs is applied by the sampler, so that a loaded path
        // holds the same points an authored one does
    }
}

inline void from_json(nlohmann::json const& json, BehaviourData& data) { DATA_REQUIRED(json, data, states); }
