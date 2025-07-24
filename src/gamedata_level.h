#pragma once

#include "dataref.h"
#include "gamedata_pickup.h"
#include "gamedata_spawner.h"
#include <canyon/utils/vector.h>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

struct LevelEvent {
    float offset = 0;
    canyon::FloatVec2 location;
    std::optional<DataRef<PickupData>> drop;
    std::optional<DataRef<SpawnerData>> spawner;
};

inline void from_json(nlohmann::json const& json, LevelEvent& data) {
    DATA_REQUIRED(json, data, offset);
    DATA_REQUIRED(json, data, location);
    DATA_OPTIONAL(json, data, drop);
    DATA_OPTIONAL(json, data, spawner);
}

struct LevelData {
    static constexpr GameDataCategory Category = GameDataCategory::Levels;

    std::vector<LevelEvent> events;
};

void from_json(nlohmann::json const& json, LevelData& data);
