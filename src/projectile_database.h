#pragma once

#include "system_projectile.h"
#include <canyon/graphics/blend_mode.h>
#include <canyon/graphics/color.h>
#include <canyon/graphics/iimage.h>
#include <canyon/graphics/surface_context.h>
#include <entt/entity/fwd.hpp>
#include <filesystem>
#include <moth_ui/utils/vector.h>
#include <nlohmann/json_fwd.hpp>

struct SpriteData {
    std::shared_ptr<canyon::graphics::IImage> image;
    moth_ui::FloatVec2 scale;
    moth_ui::IntVec2 offset;
    float rotation;
    canyon::graphics::BlendMode blend_mode;
    canyon::graphics::Color color;

    bool Load(nlohmann::json json, canyon::graphics::SurfaceContext& surfaceContext);
};

struct ProjectileData {
    std::string name;
    std::vector<SpriteData> white_srites;
    std::vector<SpriteData> black_sprites;
    float radius;
    float damage;
    float speed;

    bool Load(nlohmann::json json, canyon::graphics::SurfaceContext& surfaceContext);
};

class ProjectileDatabase {
public:
    static std::unique_ptr<ProjectileDatabase> Load(std::filesystem::path const& path,
                                                    canyon::graphics::SurfaceContext& surfaceContext);
    void SpawnProjectile(std::string const& name, entt::registry& registry, entt::entity source,
                         moth_ui::FloatVec2 position, moth_ui::FloatVec2 direction, EnergyColor color,
                         Team team, uint32_t lifetime);

private:
    ProjectileDatabase() = default;

    std::map<std::string, ProjectileData> m_database;
};
