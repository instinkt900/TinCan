#include "gamedata_spawner.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpawnerData, enemy_name, behaviour_name, count, cooldown, group_count, group_delay);

SpawnerData SpawnerData::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
    SpawnerData data = json.get<SpawnerData>();

    auto parameterList = json.value("behaviour_parameters", nlohmann::json::object());
    if (!parameterList.is_null()) {
        if (!parameterList.is_object()) {
            spdlog::error("Malformed behavious parameter list");
        } else {
            for (auto [optionName, optionJson] : parameterList.items()) {
                if (optionJson.is_string()) {
                    data.behaviour_parameters.insert(std::make_pair(optionName, optionJson.get<std::string>()));
                } else if (optionJson.is_number_integer()) {
                    data.behaviour_parameters.insert(std::make_pair(optionName, optionJson.get<std::int32_t>()));
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
