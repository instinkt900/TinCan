#pragma once

#include <map>
#include <nlohmann/detail/macro_scope.hpp>
#include <string>
#include <variant>
#include "gamedata_enemy.h"
#include "gamedata_behaviour.h"
#include "utils_paths.h"
#include "utils_serialization.h"

enum class SpawnerType {
    Unknown = -1,
    Basic,
    Staggered,
    Stepped,
    Formation,
};
MAGIC_SERIALIZE_ENUM(SpawnerType);

enum class EnemyKillType {
    Unknown = -1,
    Time,
    Bounds,
};
MAGIC_SERIALIZE_ENUM(EnemyKillType);

using BehaviourParameterList = std::map<std::string, std::variant<std::string, float>>;

namespace nlohmann {
    template <>
    struct adl_serializer<std::variant<std::string, float>> {
        static void from_json(const json& json, std::variant<std::string, float>& data) {
            if (json.is_string()) {
                data = json.get<std::string>();
            } else if (json.is_number_float()) {
                data = json.get<float>();
            }
        }
    };
}

struct SpawnerData {
    static constexpr GameDataCategory Category = GameDataCategory::Spawners;

    DataRef<EnemyData> enemy;
    DataRef<BehaviourData> behaviour;
    int32_t count = 1;
    int32_t cooldown = 0;
    int32_t group_count = 1;
    int32_t group_delay = 0;
    SpawnerType type = SpawnerType::Basic;
    moth_graphics::FloatVec2 offset_step;
    std::vector<moth_graphics::FloatVec2> formation_positions;
    DataRef<PickupData> drop;
    EnemyKillType kill_type = EnemyKillType::Bounds;
    std::optional<uint32_t> lifetime = 0;
    std::optional<float> bounds_border = 0;
    SplineDefinition spline;
    uint32_t tick_offset = 0;
};

inline void from_json(nlohmann::json const& json, SpawnerData& data) {
    DATA_REQUIRED(json, data, enemy);
    DATA_OPTIONAL(json, data, behaviour);
    DATA_OPTIONAL(json, data, count);
    DATA_OPTIONAL(json, data, cooldown);
    DATA_OPTIONAL(json, data, group_count);
    DATA_OPTIONAL(json, data, group_delay);
    DATA_OPTIONAL(json, data, type);
    DATA_OPTIONAL(json, data, offset_step);
    DATA_OPTIONAL(json, data, formation_positions);
    DATA_OPTIONAL(json, data, drop);
    DATA_OPTIONAL(json, data, kill_type);
    DATA_OPTIONAL(json, data, lifetime);
    DATA_OPTIONAL(json, data, bounds_border);
    DATA_OPTIONAL(json, data, spline);
    DATA_OPTIONAL(json, data, tick_offset);

    if (!data.spline.empty()) {
        data.spline.insert(data.spline.begin(), data.spline[0]);
        data.spline.push_back(data.spline.back());
    }
}
