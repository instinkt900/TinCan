#pragma once

#include "gamedata_database.h"
#include "system_behaviour.h"
#include <nlohmann/json_fwd.hpp>

enum class SpawnerType {
    Unknown,
    Basic,
    Staggered,
    Stepped,
};

struct SpawnerData {
    std::string enemy_name;
    std::string behaviour_name;
    BehaviourParameterList behaviour_parameters;
    int32_t count;
    int32_t cooldown;
    int32_t group_count;
    int32_t group_delay;
    SpawnerType type;
    float distance;
    std::optional<std::string> drop;

    static SpawnerData Deserialize(nlohmann::json const& json, SerializeContext const& context);
};
