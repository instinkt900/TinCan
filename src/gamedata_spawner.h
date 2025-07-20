#pragma once

#include "gamedata_database.h"
#include <nlohmann/json_fwd.hpp>
#include <variant>

enum class SpawnerType {
    Unknown,
    Basic,
    Staggered,
    Stepped,
};

enum class EnemyBehaviour {
    Unknown = -1,
    Straight,
    Wave,
};

using BehaviourParameterList = std::map<std::string, std::variant<std::string, int32_t, float>>;

struct SpawnerData {
    std::string enemy_name;
    EnemyBehaviour behaviour;
    BehaviourParameterList behaviour_parameters;
    int32_t count;
    int32_t cooldown;
    int32_t group_count;
    int32_t group_delay;
    SpawnerType type;
    canyon::FloatVec2 offset_step;
    std::optional<std::string> drop;

    static SpawnerData Deserialize(nlohmann::json const& json, SerializeContext const& context);
};
