#include "gamedata_spawner.h"
#include "serialization_utils.h"
#include "utils.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <magic_enum.hpp>

MAGIC_SERIALIZE_ENUM(SpawnerType);
MAGIC_SERIALIZE_ENUM(EnemyBehaviour);
MAGIC_SERIALIZE_ENUM(EnemyKillType);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpawnerData, enemy_name, behaviour, count, cooldown, group_count,
                                   group_delay, type, offset_step, kill_type);

SpawnerData SpawnerData::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
    SpawnerData data = json.get<SpawnerData>();

    opt_get_to(json, "drop_name", data.drop);
    opt_get_to(json, "lifetime", data.lifetime);
    opt_get_to(json, "bounds_border", data.bounds_border);

    auto parameterList = json.value("behaviour_parameters", nlohmann::json());
    if (!parameterList.is_null()) {
        if (!parameterList.is_object()) {
            spdlog::error("Malformed behavious parameter list");
        } else {
            for (auto [optionName, optionJson] : parameterList.items()) {
                if (optionJson.is_string()) {
                    data.behaviour_parameters.insert(
                        std::make_pair(optionName, optionJson.get<std::string>()));
                } else if (optionJson.is_number_integer()) {
                    data.behaviour_parameters.insert(
                        std::make_pair(optionName, optionJson.get<std::int32_t>()));
                } else if (optionJson.is_number_float()) {
                    data.behaviour_parameters.insert(std::make_pair(optionName, optionJson.get<float>()));
                } else {
                    spdlog::error("Unknown behaviour parameter type for {}", optionName);
                    continue;
                }
            }
        }
    }
    return data;
}
