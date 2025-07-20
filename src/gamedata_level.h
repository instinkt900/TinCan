#pragma once

#include "gamedata_database.h"
#include "gamedata_spawner.h"
#include <canyon/utils/vector.h>
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

struct LevelEvent {
    float offset = 0;
    canyon::FloatVec2 location;
    std::optional<std::string> drop_name;
    std::optional<SpawnerData> spawner;
};

struct LevelData {
    std::vector<LevelEvent> events;

    static SerializeContext const* CurrentContext;
    static LevelData Deserialize(nlohmann::json const& json, SerializeContext const& context);
};

