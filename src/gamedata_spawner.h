#pragma once

#include <map>
#include <nlohmann/detail/macro_scope.hpp>
#include <string>
#include <variant>
#include "gamedata_enemy.h"
#include "utils_paths.h"
#include "utils_serialization.h"

enum class SpawnerType {
    Unknown = -1,
    Basic,
    Staggered,
    Stepped,
};
MAGIC_SERIALIZE_ENUM(SpawnerType);

enum class EnemyBehaviour {
    Unknown = -1,
    Straight,
    Wave,
    Spline,
};
MAGIC_SERIALIZE_ENUM(EnemyBehaviour);

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
    EnemyBehaviour behaviour;
    BehaviourParameterList behaviour_parameters;
    int32_t count;
    int32_t cooldown;
    int32_t group_count;
    int32_t group_delay;
    SpawnerType type;
    canyon::FloatVec2 offset_step;
    std::optional<DataRef<PickupData>> drop;
    EnemyKillType kill_type;
    std::optional<uint32_t> lifetime;
    std::optional<float> bounds_border;
    SplineDefinition spline;
    uint32_t tick_offset = 0;
};

inline void from_json(nlohmann::json const& json, SpawnerData& data) {
    DATA_REQUIRED(json, data, enemy);
    DATA_REQUIRED(json, data, behaviour);
    DATA_REQUIRED(json, data, behaviour_parameters);
    DATA_REQUIRED(json, data, count);
    DATA_REQUIRED(json, data, cooldown);
    DATA_REQUIRED(json, data, group_count);
    DATA_REQUIRED(json, data, group_delay);
    DATA_REQUIRED(json, data, type);
    DATA_REQUIRED(json, data, offset_step);
    DATA_OPTIONAL(json, data, drop);
    DATA_REQUIRED(json, data, kill_type);
    DATA_OPTIONAL(json, data, lifetime);
    DATA_OPTIONAL(json, data, bounds_border);
    DATA_OPTIONAL(json, data, spline);
    DATA_OPTIONAL(json, data, tick_offset);

    if (!data.spline.empty()) {
        data.spline.insert(data.spline.begin(), data.spline[0]);
        data.spline.push_back(data.spline.back());
    }
}
