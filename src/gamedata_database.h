#pragma once

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

template <typename T>
class Database {
public:
    bool Load(nlohmann::json const& json) {
        if (!json.is_object()) {
            return false;
        }

        for (auto [entryName, entryJson]: json.items()) {
            m_database.insert(std::make_pair(entryName, entryJson.get<T>()));
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

