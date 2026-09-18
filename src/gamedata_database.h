#pragma once

#include <magic_enum.hpp>
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
            // A bad entry is skipped rather than taken as fatal: the rest of the
            // database still loads, and anything referring to the missing entry
            // reports it when the reference is resolved.
            try {
                m_database.insert(std::make_pair(entryName, entryJson.get<T>()));
            } catch (std::exception const& e) {
                spdlog::error("Skipping {} entry {}: {}", magic_enum::enum_name(T::Category), entryName,
                              e.what());
            }
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

    std::vector<std::string> Keys() const {
        std::vector<std::string> keys;
        keys.reserve(m_database.size());

        for (auto const& [key, value] : m_database) {
            keys.push_back(key);
        }

        return keys;
    }

    size_t Size() const { return m_database.size(); }

private:
    std::map<std::string, T> m_database;
};

