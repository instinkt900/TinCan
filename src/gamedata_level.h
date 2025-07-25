#pragma once

#include "dataref.h"
#include "gamedata_pickup.h"
#include "gamedata_spawner.h"
#include <canyon/utils/vector.h>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>
#include <fstream>

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

inline void from_json(nlohmann::json const& json, LevelData& data) {
    // leveldata comes in as a string reference to an external file
    if (!json.is_string()) {
        spdlog::error("Invalid level data entry type.");
        throw std::runtime_error("Invalid level data entry type.");
    }

    std::filesystem::path const path = json.get<std::string>();

    if (!std::filesystem::exists(path)) {
        spdlog::error("Level data does not exist at {}", path.string());
        throw std::runtime_error("Level data does not exist at " + path.string());
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::error("Unable to open level file at {}", path.string());
        throw std::runtime_error("Unable to open level file at " + path.string());
    }

    nlohmann::json levelJson;
    try {
        file >> levelJson;
    } catch (std::exception& e) {
        spdlog::error("Failed to load level json at {}", path.string());
        spdlog::error("{}", e.what());
        throw std::runtime_error("Failed to load level json at " + path.string());
    }

    levelJson["events"].get_to(data.events);
}

