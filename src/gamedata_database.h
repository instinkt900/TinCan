#pragma once

#include <canyon/graphics/surface_context.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>

class Gamedata;

struct SerializeContext {
    Gamedata const& gamedata;
    canyon::graphics::SurfaceContext& surfaceContext;
};

template <typename T> class Database {
public:
    static std::unique_ptr<Database<T>> Load(std::filesystem::path const& path, SerializeContext const& context) {
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

        if (!json.is_object()) {
            spdlog::error("Malformed database at {}. Root is not an object.", path.string());
            return nullptr;
        }

        std::unique_ptr<Database<T>> db = std::unique_ptr<Database<T>>(new Database<T>());
        for (auto [entryName, entryJson]: json.items()) {
            db->m_database.insert(std::make_pair(entryName, T::Deserialize(entryJson, context)));
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

