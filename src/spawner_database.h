#pragma once

#include <filesystem>
#include <map>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <variant>

struct SpawnerData {
    std::string name;
    std::string enemy_name;
    std::string behaviour_name;
    std::map<std::string, std::variant<std::string, int32_t, float>> behaviour_parameters;
    int32_t count;
    int32_t cooldown;
    int32_t group_count;
    int32_t group_delay;

    bool Load(nlohmann::json const& json);
};

class SpawnerDatabase {
    public:
        static std::unique_ptr<SpawnerDatabase> Load(std::filesystem::path const& path);
        SpawnerData const* GetSpawnerData(std::string const& name) const;

    private:
        SpawnerDatabase() = default;

        std::map<std::string, SpawnerData> m_database;
};
