#include "weapon_database.h"
#include <spdlog/spdlog.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <moth_ui/utils/vector_serialization.h>

bool WeaponData::Load(nlohmann::json const& json) {
    json["name"].get_to(name);
    json["cooldown"].get_to(cooldown);
    json["player_tracking"].get_to(player_tracking);
    json["projectile_name"].get_to(projectile_name);
    for (auto const& barrelJson: json["barrels"]) {
        auto& barrel = barrels.emplace_back();
        barrelJson["offset"].get_to(barrel.offset);
        barrelJson["group"].get_to(barrel.group);
    }

    return true;
}

std::unique_ptr<WeaponDatabase> WeaponDatabase::Load(std::filesystem::path const& path,
                                                   SpriteDatabase const& spriteDatabase) {
    if (!std::filesystem::exists(path)) {
        spdlog::error("Weapon database does not exist at {}", path.string());
        return nullptr;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::error("Unable to open the weapon database at {}", path.string());
        return nullptr;
    }

    nlohmann::json json;
    try {
        file >> json;
    } catch (std::exception& e) {
        spdlog::error("Failed to load weapon database at {}", path.string());
        spdlog::error("{}", e.what());
        return nullptr;
    }

    std::unique_ptr<WeaponDatabase> db = std::unique_ptr<WeaponDatabase>(new WeaponDatabase(spriteDatabase));
    for (auto weaponJson : json) {
        WeaponData weaponData;
        if (weaponData.Load(weaponJson)) {
            db->m_database.insert(std::make_pair(weaponData.name, weaponData));
        }
    }

    spdlog::info("Loaded weapon database: {}", path.string());
    return db;
}

WeaponData const* WeaponDatabase::GetWeaponData(std::string const& name) const {
    auto entry = m_database.find(name);
    if (entry == m_database.end()) {
        spdlog::error("Cannot find weapon name {}", name);
        return nullptr;
    }
    return &entry->second;
}

