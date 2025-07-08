#pragma once

#include "gamedata_database.h"
#include <moth_ui/utils/vector.h>
#include <nlohmann/json_fwd.hpp>
#include <vector>

enum class LevelEventType {
    Unknown,
    Spawn,
    CreateSpawner,
};

struct LevelEvent {
    float time;
    LevelEventType type;
    moth_ui::FloatVec2 location;
    std::string name;
};

struct LevelData {
    std::vector<LevelEvent> events;

    static LevelData Deserialize(nlohmann::json const& json, SerializeContext const& context);
};

