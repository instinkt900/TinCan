#pragma once

#include <canyon/graphics/surface_context.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>

class Gamedata;

template <typename T> class Database {
public:
    static std::unique_ptr<Database<T>> Load(std::filesystem::path const& path, Gamedata const& databases,
                                             canyon::graphics::SurfaceContext& surfaceContext) {
        if (!std::filesystem::exists(path)) {
            spdlog::error("Database does not exist at {}", path.string());
            return nullptr;
        }

        std::ifstream file(path);
        if (!file.is_open()) {
            spdlog::error("Unable to open the database at {}", path.string());
            return nullptr;
        }

        nlohmann::json json;
        try {
            file >> json;
        } catch (std::exception& e) {
            spdlog::error("Failed to load database at {}", path.string());
            spdlog::error("{}", e.what());
            return nullptr;
        }

        std::unique_ptr<Database<T>> db = std::unique_ptr<Database<T>>(new Database<T>());
        for (auto entryJson : json) {
            T data;
            if (data.Load(entryJson, databases, surfaceContext)) {
                db->m_database.insert(std::make_pair(data.name, data));
            }
        }

        spdlog::info("Loaded database: {}", path.string());
        return db;
    }

    T const* Get(std::string const& name) const {
        auto entry = m_database.find(name);
        if (entry == m_database.end()) {
            spdlog::error("Unable to find data name {}", name);
            return nullptr;
        }
        return &entry->second;
    }

private:
    Database() = default;

    std::map<std::string, T> m_database;
};

