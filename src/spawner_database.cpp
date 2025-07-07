#include "spawner_database.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>

bool SpawnerData::Load(nlohmann::json const& json) {
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

std::unique_ptr<SpawnerDatabase> SpawnerDatabase::Load(std::filesystem::path const& path) {
    if (!std::filesystem::exists(path)) {
        spdlog::error("Spawner database does not exist at {}", path.string());
        return nullptr;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::error("Unable to open the spawner database at {}", path.string());
        return nullptr;
    }

    nlohmann::json json;
    try {
        file >> json;
    } catch (std::exception& e) {
        spdlog::error("Failed to load spawner database at {}", path.string());
        spdlog::error("{}", e.what());
        return nullptr;
    }

    std::unique_ptr<SpawnerDatabase> db = std::unique_ptr<SpawnerDatabase>(new SpawnerDatabase());
    for (auto spawnerJson : json) {
        SpawnerData spawnerData;
        if (spawnerData.Load(spawnerJson)) {
            db->m_database.insert(std::make_pair(spawnerData.name, spawnerData));
        }
    }

    spdlog::info("Loaded spawner database: {}", path.string());
    return db;
}

SpawnerData const* SpawnerDatabase::GetSpawnerData(std::string const& name) const {
    auto entry = m_database.find(name);
    if (entry == m_database.end()) {
        spdlog::error("Cannot find spawner name {}", name);
        return nullptr;
    }
    return &entry->second;
}
