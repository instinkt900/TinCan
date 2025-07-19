#include "gamedata.h"
#include <fstream>

MAGIC_SERIALIZE_ENUM(GameDataCategory);

void GameData::LoadDirectory(std::filesystem::path const& directory, SerializeContext const& context) {
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            Load(entry.path(), context);
        }
    }
}

bool GameData::Load(std::filesystem::path const& path, SerializeContext const& context) {
    if (!std::filesystem::exists(path)) {
        spdlog::error("Database does not exist at {}", path.string());
        return false;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::error("Unable to open the database at {}", path.string());
        return false;
    }

    nlohmann::json json;
    try {
        file >> json;
    } catch (std::exception& e) {
        spdlog::error("Failed to load database at {}", path.string());
        spdlog::error("{}", e.what());
        return false;
    }

    if (!json.is_object()) {
        spdlog::error("Malformed database at {}. Root is not an object.", path.string());
        return false;
    }

    magic_enum::enum_for_each<GameDataCategory>([&](GameDataCategory category) {
        auto const categoryName = magic_enum::enum_name(category);
        auto const dbJson = json.value(categoryName, nlohmann::json());
        if (dbJson.is_null()) {
            return;
        }
        switch (category) {
        case GameDataCategory::Sprites:
            GetSpriteDatabase().Load(dbJson, context);
            break;
        case GameDataCategory::Projectiles:
            GetProjectileDatabase().Load(dbJson, context);
            break;
        case GameDataCategory::Weapons:
            GetWeaponDatabase().Load(dbJson, context);
            break;
        case GameDataCategory::Enemies:
            GetEnemyDatabase().Load(dbJson, context);
            break;
        case GameDataCategory::Spawners:
            GetSpawnerDatabase().Load(dbJson, context);
            break;
        case GameDataCategory::Levels:
            GetLevelDatabase().Load(dbJson, context);
            break;
        case GameDataCategory::Pickups:
            GetPickupDatabase().Load(dbJson, context);
            break;
        }
    });

    spdlog::info("Loaded database: {}", path.string());
    return true;
}
