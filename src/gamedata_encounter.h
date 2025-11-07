#pragma once

#include "gamedata.h"
#include "gamedata_level.h"
#include "utils_serialization.h"

struct EncounterData {
    static constexpr GameDataCategory Category = GameDataCategory::Encounters;
    
    float duration;
    std::vector<LevelEvent> events;
};

inline void from_json(nlohmann::json const& json, EncounterData& data) {
    DATA_REQUIRED(json, data, duration);
    DATA_REQUIRED(json, data, events);
}

