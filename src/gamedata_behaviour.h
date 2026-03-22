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
    moth_graphics::FloatVec2 velocity{ 0, 0 };
    TargetType target{ TargetType::Undefined };
    moth_graphics::FloatVec2 point{ 0, 0 };
    std::vector<moth_graphics::FloatVec2> points;
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
        // duplicate the start and end points to get smooth transitions
        data.points.insert(data.points.begin(), data.points[0]);
        data.points.push_back(data.points.back());
    }
}

inline void from_json(nlohmann::json const& json, BehaviourData& data) { DATA_REQUIRED(json, data, states); }
