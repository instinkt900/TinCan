#pragma once

#include "gamedata.h"
#include "utils_serialization.h"

// A formation is the largest version of a shape, as an ordered list of slot offsets in unit space. A
// spawn's count fills the slots one by one from the front, so one entry serves every count up to its
// size and the shape fills out as the count grows. Slots are used where they are authored: the subset a
// smaller count uses is not re-centred on the spawn point.
struct FormationData {
    static constexpr GameDataCategory Category = GameDataCategory::Formations;

    std::vector<moth::gfx::FloatVec2> slots;
};

inline void from_json(nlohmann::json const& json, FormationData& data) {
    DATA_REQUIRED(json, data, slots);
}

inline void to_json(nlohmann::json& json, FormationData const& data) {
    json["slots"] = data.slots;
}
