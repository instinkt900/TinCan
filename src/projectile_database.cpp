#include "projectile_database.h"
#include "game_layer.h"
#include "system_drawable.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_projectile.h"
#include <canyon/graphics/color.h>
#include <canyon/graphics/iimage.h>
#include <canyon/graphics/surface_context.h>
#include <moth_ui/utils/vector.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <moth_ui/utils/vector_serialization.h>
#include <canyon/utils/vector_serialization.h>
#include <entt/entt.hpp>

bool ProjectileData::Load(nlohmann::json json, SpriteDatabase const& spriteDatabase) {
    auto const* whiteSpriteData = spriteDatabase.GetSpriteData(json["white_sprite"]);
    if (whiteSpriteData != nullptr) {
        white_sprite = *whiteSpriteData;
    }
    auto const* blackSpriteData = spriteDatabase.GetSpriteData(json["black_sprite"]);
    if (blackSpriteData != nullptr) {
        black_sprite = *blackSpriteData;
    }

    json["name"].get_to(name);
    json["radius"].get_to(radius);
    json["damage"].get_to(damage);
    json["speed"].get_to(speed);

    return true;
}

std::unique_ptr<ProjectileDatabase> ProjectileDatabase::Load(std::filesystem::path const& path,
                                                             SpriteDatabase const& spriteDatabase) {
    if (!std::filesystem::exists(path)) {
        spdlog::error("Projectile database does not exist at {}", path.string());
        return nullptr;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::error("Unable to open the projectile database at {}", path.string());
        return nullptr;
    }

    nlohmann::json json;
    try {
        file >> json;
    } catch (std::exception& e) {
        spdlog::error("Failed to load projectile database at {}", path.string());
        spdlog::error("{}", e.what());
        return nullptr;
    }

    std::unique_ptr<ProjectileDatabase> db =
        std::unique_ptr<ProjectileDatabase>(new ProjectileDatabase(spriteDatabase));
    for (auto projectileJson : json) {
        ProjectileData projectileData;
        if (projectileData.Load(projectileJson, spriteDatabase)) {
            db->m_database.insert(std::make_pair(projectileData.name, projectileData));
        }
    }

    spdlog::info("Loaded projectile database: {}", path.string());
    return db;
}

ProjectileData const* ProjectileDatabase::GetProjectileData(std::string const& name) const {
    auto entry = m_database.find(name);
    if (entry == m_database.end()) {
        spdlog::error("Unable to find projectile name {}", name);
        return nullptr;
    }
    return &entry->second;
}
