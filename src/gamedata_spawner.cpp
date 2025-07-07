#include "gamedata_spawner.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

bool SpawnerData::Load(nlohmann::json const& json, Gamedata const& gamedata, canyon::graphics::SurfaceContext& surfaceContext) {
    json["name"].get_to(name);
    json["enemy_name"].get_to(enemy_name);
    json["behaviour_name"].get_to(behaviour_name);
    json["count"].get_to(count);
    json["cooldown"].get_to(cooldown);
    json["group_count"].get_to(group_count);
    json["group_delay"].get_to(group_delay);

    auto parameterList = json.value("behaviour_parameters", nlohmann::json::object());
    if (!parameterList.is_null()) {
        if (!parameterList.is_object()) {
            spdlog::error("Malformed behavious parameter list for {}", name);
        } else {
            for (auto [optionName, optionJson] : parameterList.items()) {
                if (optionJson.is_string()) {
                    behaviour_parameters.insert(std::make_pair(optionName, optionJson.get<std::string>()));
                } else if (optionJson.is_number_integer()) {
                    behaviour_parameters.insert(std::make_pair(optionName, optionJson.get<std::int32_t>()));
                } else if (optionJson.is_number_float()) {
                    behaviour_parameters.insert(std::make_pair(optionName, optionJson.get<float>()));
                } else {
                    spdlog::error("Unknown behaviour parameter type for {}", optionName);
                    continue;
                }
            }
        }
    }
    return true;
}

