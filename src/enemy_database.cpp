#include "enemy_database.h"
#include <spdlog/spdlog.h>
#include <fstream>
#include <nlohmann/json.hpp>

bool EnemyData::Load(nlohmann::json const& json) {
    json["name"].get_to(name);
    json["sprite_name"].get_to(sprite_name);
    json["health"].get_to(health);
    json["radius"].get_to(radius);
    json["lifetime"].get_to(lifetime);
    json["weapon_name"].get_to(weapon_name);
    return true;
}

std::unique_ptr<EnemyDatabase> EnemyDatabase::Load(std::filesystem::path const& path,
                                                   SpriteDatabase const& spriteDatabase) {
    if (!std::filesystem::exists(path)) {
        spdlog::error("Enemy database does not exist at {}", path.string());
        return nullptr;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::error("Unable to open the enemy database at {}", path.string());
        return nullptr;
    }

    nlohmann::json json;
    try {
        file >> json;
    } catch (std::exception& e) {
        spdlog::error("Failed to load enemy database at {}", path.string());
        spdlog::error("{}", e.what());
        return nullptr;
    }

    std::unique_ptr<EnemyDatabase> db = std::unique_ptr<EnemyDatabase>(new EnemyDatabase(spriteDatabase));
    for (auto enemyJson : json) {
        EnemyData enemyData;
        if (enemyData.Load(enemyJson)) {
            db->m_database.insert(std::make_pair(enemyData.name, enemyData));
        }
    }

    spdlog::info("Loaded enemy database: {}", path.string());
    return db;
}

EnemyData const* EnemyDatabase::GetEnemyData(std::string const& name) const {
    auto entry = m_database.find(name);
    if (entry == m_database.end()) {
        spdlog::error("Cannot find enemy name {}", name);
        return nullptr;
    }
    return &entry->second;
}
