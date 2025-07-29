#pragma once

#include "gamedata.h"
#include "gamedata_entity.h"

struct EnemyData {
    static constexpr GameDataCategory Category = GameDataCategory::Enemies;

    Affinity affinity = Affinity::None;
    std::map<std::string, EntitySection> body;
};

inline void from_json(nlohmann::json const& json, EnemyData& data) {
    DATA_OPTIONAL(json, data, affinity);
    DATA_REQUIRED(json, data, body);
}

