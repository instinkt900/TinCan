#include "gamedata.h"
#include <fstream>
#include "utils_serialization.h"
#include "gamedata_sprite.h"
#include "gamedata_enemy.h"
#include "gamedata_level.h"
#include "gamedata_pickup.h"
#include "gamedata_projectile.h"
#include "gamedata_spawner.h"
#include "gamedata_weapon.h"

MAGIC_SERIALIZE_ENUM(GameDataCategory);

SerializeContext const* GameData::s_currentContext = nullptr;

std::string ToLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
}

GameData::GameData() = default;
GameData::~GameData() = default;

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

    s_currentContext = &context;
    for (auto [categoryName, categoryJson] : json.items()) {
        auto const category =
            magic_enum::enum_cast<GameDataCategory>(categoryName, magic_enum::case_insensitive);
        if (!category.has_value()) {
            spdlog::error("Unknown data category {}", categoryName);
            continue;
        }
        switch (category.value()) {
        case GameDataCategory::Sprites:
            m_spriteDatabase.Load(categoryJson);
            break;
        case GameDataCategory::Projectiles:
            m_projectileDatabase.Load(categoryJson);
            break;
        case GameDataCategory::Weapons:
            m_weaponDatabase.Load(categoryJson);
            break;
        case GameDataCategory::Enemies:
            m_enemyDatabase.Load(categoryJson);
            break;
        case GameDataCategory::Spawners:
            m_spawnerDatabase.Load(categoryJson);
            break;
        case GameDataCategory::Levels:
            m_levelDatabase.Load(categoryJson);
            break;
        case GameDataCategory::Pickups:
            m_pickupDatabase.Load(categoryJson);
            break;
        default:
            break;
        }
    }
    s_currentContext = nullptr;

    spdlog::info("Loaded database: {}", path.string());
    return true;
}
