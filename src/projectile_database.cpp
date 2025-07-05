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

bool SpriteData::Load(nlohmann::json json, canyon::graphics::SurfaceContext& surfaceContext) {
    std::filesystem::path imagePath;
    json["image"].get_to(imagePath);
    image = surfaceContext.ImageFromFile(imagePath);
    if (image == nullptr) {
        spdlog::error("Failed loading sprite at {}", imagePath.string());
        return false;
    }
    json["scale"].get_to(scale);
    json["offset"].get_to(offset);
    json["rotation"].get_to(rotation);
    json["blend_mode"].get_to(blend_mode);
    json["color"].get_to(color);
    return true;
}

bool ProjectileData::Load(nlohmann::json json, canyon::graphics::SurfaceContext& surfaceContext) {
    for (auto spriteJson : json["white_sprites"]) {

        SpriteData sprite;
        if (sprite.Load(spriteJson, surfaceContext)) {
            white_srites.push_back(sprite);
        }
    }
    for (auto spriteJson : json["black_sprites"]) {
        SpriteData sprite;
        if (sprite.Load(spriteJson, surfaceContext)) {
            black_sprites.push_back(sprite);
        }
    }

    json["name"].get_to(name);
    json["radius"].get_to(radius);
    json["damage"].get_to(damage);
    json["speed"].get_to(speed);

    return true;
}

std::unique_ptr<ProjectileDatabase>
ProjectileDatabase::Load(std::filesystem::path const& path,
                         canyon::graphics::SurfaceContext& surfaceContext) {
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

    std::unique_ptr<ProjectileDatabase> db = std::unique_ptr<ProjectileDatabase>(new ProjectileDatabase());
    for (auto projectileJson : json) {
        ProjectileData projectileData;
        if (projectileData.Load(projectileJson, surfaceContext)) {
            db->m_database.insert(std::make_pair(projectileData.name, projectileData));
        }
    }

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

    auto& sprites = color == EnergyColor::WHITE ? entry->second.white_srites : entry->second.black_sprites;
    for (auto& spriteData : sprites) {
        auto& sprite = projectileDrawable.m_sprites.emplace_back();
        sprite.m_image = spriteData.image;
        moth_ui::FloatVec2 const imageSize = { spriteData.image->GetWidth(), spriteData.image->GetHeight() };
        sprite.m_size = static_cast<moth_ui::IntVec2>(imageSize * spriteData.scale);
        sprite.m_blendMode = spriteData.blend_mode;
        sprite.m_color = spriteData.color;
    }
    projectileLifetime.m_lifetime = lifetime;
}
