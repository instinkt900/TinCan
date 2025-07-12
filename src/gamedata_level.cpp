#include "gamedata_level.h"
#include <magic_enum.hpp>
#include <moth_ui/utils/vector_serialization.h>
#include <nlohmann/json.hpp>

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LevelData, events);

SerializeContext const* LevelData::CurrentContext = nullptr;

void from_json(nlohmann::json const& j, LevelEvent& levelEvent) {
    j["time"].get_to(levelEvent.time);
    j["location"].get_to(levelEvent.location);

    auto const eventType = j.value("type", nlohmann::json());
    if (!eventType.is_null()) {
        levelEvent.type = eventType;
    }

    auto const eventName = j.value("name", nlohmann::json());
    if (!eventName.is_null()) {
        levelEvent.name = eventName;
    }

    auto const dropName = j.value("drop_name", nlohmann::json());
    if (!dropName.is_null()) {
        dropName.get_to(levelEvent.drop_name);
    }

    auto const spawner = j.value("spawner", nlohmann::json());
    if (!spawner.is_null()) {
        assert(LevelData::CurrentContext != nullptr);
        levelEvent.spawner = SpawnerData::Deserialize(spawner, *LevelData::CurrentContext);
    }
}

LevelData LevelData::Deserialize(nlohmann::json const& json, SerializeContext const& context) {
    CurrentContext = &context;

    // leveldata comes in as a string reference to an external file
    if (!json.is_string()) {
        spdlog::error("Invalid level data entry type.");
        throw std::runtime_error("Invalid level data entry type.");
    }

    std::filesystem::path const path = json.get<std::string>();

    if (!std::filesystem::exists(path)) {
        spdlog::error("Level data does not exist at {}", path.string());
        throw std::runtime_error("Level data does not exist at " + path.string());
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::error("Unable to open level file at {}", path.string());
        throw std::runtime_error("Unable to open level file at " + path.string());
    }

    nlohmann::json levelJson;
    try {
        file >> levelJson;
    } catch (std::exception& e) {
        spdlog::error("Failed to load level json at {}", path.string());
        spdlog::error("{}", e.what());
        throw std::runtime_error("Failed to load level json at " + path.string());
    }

    return levelJson.get<LevelData>();
}
