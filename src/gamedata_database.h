#pragma once

#include <canyon/graphics/surface_context.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

class GameData;

struct SerializeContext {
    GameData const& gamedata;
    canyon::graphics::SurfaceContext& surfaceContext;
};

template <typename T> class Database {
public:
    bool Load(nlohmann::json const& json, SerializeContext const& context) {
        if (!json.is_object()) {
            return false;
        }

        for (auto [entryName, entryJson]: json.items()) {
            m_database.insert(std::make_pair(entryName, T::Deserialize(entryJson, context)));
        }

        return true;
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
    std::map<std::string, T> m_database;
};

