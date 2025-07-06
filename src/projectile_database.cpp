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

void ProjectileDatabase::SpawnProjectile(std::string const& name, entt::registry& registry,
                                         entt::entity source, moth_ui::FloatVec2 position,
                                         moth_ui::FloatVec2 direction, EnergyColor color, Team team,
                                         uint32_t lifetime) {
    auto entry = m_database.find(name);
    if (entry == m_database.end()) {
        spdlog::error("Unable to find projectile name {}", name);
        return;
    }

    auto projectile = registry.create();
    auto& projectileData = registry.emplace<ComponentProjectile>(projectile);
    auto& projectileEntityData = registry.emplace<ComponentEntity>(projectile);
    auto& projectilePos = registry.emplace<ComponentPosition>(projectile);
    auto& projectileVel = registry.emplace<ComponentVelocity>(projectile);
    auto& projectileDrawable = registry.emplace<ComponentDrawable>(projectile);
    auto& projectileLifetime = registry.emplace<ComponentLifetime>(projectile);

    projectileData.m_owner = source;
    projectileData.m_color = color;
    projectileData.m_damage = entry->second.damage;
    projectileEntityData.m_team = team;
    projectileEntityData.m_radius = entry->second.radius;
    projectilePos.m_position = position;
    projectileVel.m_velocity = direction * entry->second.speed;

    auto& sprite = color == EnergyColor::WHITE ? entry->second.white_sprite : entry->second.black_sprite;
    projectileDrawable.m_spriteData = sprite;
    projectileLifetime.m_msLeft = lifetime;
}
