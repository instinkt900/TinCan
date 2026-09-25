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

// PathInterpolation and TransitionType belong to the sampler, which is what gives them meaning
MAGIC_SERIALIZE_ENUM(PathInterpolation);
MAGIC_SERIALIZE_ENUM(TransitionType);

// What the coordinates of a behaviour are measured against. A path is usually authored as a shape relative
// to wherever the enemy comes in, which is what SpawnRelative means.
enum class BehaviourSpace {
    SpawnRelative,
    World,
};
MAGIC_SERIALIZE_ENUM(BehaviourSpace);

// The screen edge a behaviour enters from. Paths are authored as they play from that edge; the edge only
// decides where a spawn places the formation, and which way the formation's offsets turn.
enum class BehaviourEdge {
    Top,
    Bottom,
    Left,
    Right,
};
MAGIC_SERIALIZE_ENUM(BehaviourEdge);

// What a modifier does to the position a path produced. A fixed, small set of numbers: modifiers do not
// read the world, do not branch and cannot be nested.
enum class ModifierType {
    Undefined,
    // offsets the cycle of the wave, so entities spawned together do not move in lockstep
    Phase,
    // reshapes speed along the path without bending its geometry
    Ease,
    // oscillates across the direction of travel
    Wave,
};
MAGIC_SERIALIZE_ENUM(ModifierType);

enum class EaseType {
    In,
    Out,
    InOut,
};
MAGIC_SERIALIZE_ENUM(EaseType);

struct BehaviourModifierData {
    ModifierType type{ ModifierType::Undefined };
    // Wave
    float amplitude{ 0 };
    float frequency{ 0 }; // cycles over the whole path
    // Phase
    float cycles{ 0 }; // cycles of offset per place in the formation
    // Ease
    EaseType ease{ EaseType::InOut };
};

inline void from_json(nlohmann::json const& json, BehaviourModifierData& data) {
    DATA_REQUIRED(json, data, type);

    if (data.type == ModifierType::Wave) {
        DATA_REQUIRED(json, data, amplitude);
        DATA_REQUIRED(json, data, frequency);
    } else if (data.type == ModifierType::Phase) {
        DATA_REQUIRED(json, data, cycles);
    } else if (data.type == ModifierType::Ease) {
        DATA_OPTIONAL(json, data, ease);
    }
}

inline void to_json(nlohmann::json& json, BehaviourModifierData const& data) {
    json["type"] = data.type;

    if (data.type == ModifierType::Wave) {
        json["amplitude"] = data.amplitude;
        json["frequency"] = data.frequency;
    } else if (data.type == ModifierType::Phase) {
        json["cycles"] = data.cycles;
    } else if (data.type == ModifierType::Ease) {
        json["ease"] = data.ease;
    }
}

// How a state reaches its own start, which is also how the first state reaches its start from wherever
// the entity spawned. It runs before the state's duration starts counting, so a short duration cannot
// silently swallow the approach.
struct BehaviourTransitionData {
    TransitionType type{ TransitionType::Snap };
    // 0 means travel at the speed of the state being entered
    float speed{ 0 };
};

inline void from_json(nlohmann::json const& json, BehaviourTransitionData& data) {
    DATA_REQUIRED(json, data, type);
    DATA_OPTIONAL(json, data, speed);
}

inline void to_json(nlohmann::json& json, BehaviourTransitionData const& data) {
    json["type"] = data.type;
    if (data.speed > 0.0f) {
        json["speed"] = data.speed;
    }
}

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
    BehaviourTransitionData entry;
    std::vector<BehaviourModifierData> modifiers;
    // whether the entity's weapons fire while this state runs
    bool fire{ true };
};

struct BehaviourData {
    static constexpr GameDataCategory Category = GameDataCategory::Behaviours;

    BehaviourSpace space{ BehaviourSpace::SpawnRelative };
    BehaviourEdge edge{ BehaviourEdge::Top };
    std::vector<BehaviourStateData> states;
    // The index of the state the behaviour goes back to after its last state, forever, or -1 for none.
    // Looping from a later state lets the states before it play once as an intro.
    int loop_from{ -1 };
};

inline void from_json(nlohmann::json const& json, BehaviourStateData& data) {
    DATA_REQUIRED(json, data, type);
    DATA_OPTIONAL(json, data, label);
    DATA_OPTIONAL(json, data, duration);
    DATA_OPTIONAL(json, data, fire);

    if (data.type == BehaviourType::Path) {
        DATA_REQUIRED(json, data, waypoints);
        DATA_OPTIONAL(json, data, interpolation);
        DATA_REQUIRED(json, data, speed);
        DATA_OPTIONAL(json, data, entry);
        DATA_OPTIONAL(json, data, modifiers);
    } else if (data.type == BehaviourType::Target) {
        DATA_REQUIRED(json, data, target);
        DATA_OPTIONAL(json, data, point);
        DATA_OPTIONAL(json, data, speed);
    }
}

inline void from_json(nlohmann::json const& json, BehaviourData& data) {
    DATA_OPTIONAL(json, data, space);
    DATA_OPTIONAL(json, data, edge);
    DATA_REQUIRED(json, data, states);
    DATA_OPTIONAL(json, data, loop_from);
}

inline void to_json(nlohmann::json& json, BehaviourStateData const& data) {
    json["type"] = data.type;
    if (!data.label.empty()) {
        json["label"] = data.label;
    }
    if (data.duration > 0) {
        json["duration"] = data.duration;
    }
    if (!data.fire) {
        json["fire"] = data.fire;
    }

    if (data.type == BehaviourType::Path) {
        json["waypoints"] = data.waypoints;
        json["interpolation"] = data.interpolation;
        json["speed"] = data.speed;
        if (data.entry.type != TransitionType::Snap) {
            json["entry"] = data.entry;
        }
        if (!data.modifiers.empty()) {
            json["modifiers"] = data.modifiers;
        }
    } else if (data.type == BehaviourType::Target) {
        json["target"] = data.target;
        json["point"] = data.point;
        json["speed"] = data.speed;
    }
}

inline void to_json(nlohmann::json& json, BehaviourData const& data) {
    json["space"] = data.space;
    if (data.edge != BehaviourEdge::Top) {
        json["edge"] = data.edge;
    }
    json["states"] = data.states;
    if (data.loop_from >= 0) {
        json["loop_from"] = data.loop_from;
    }
}
