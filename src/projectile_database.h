#pragma once

#include "sprite_database.h"
#include "system_projectile.h"
#include <canyon/graphics/blend_mode.h>
#include <canyon/graphics/color.h>
#include <canyon/graphics/iimage.h>
#include <canyon/graphics/surface_context.h>
#include <entt/entity/fwd.hpp>
#include <filesystem>
#include <moth_ui/utils/vector.h>
#include <nlohmann/json_fwd.hpp>

struct ProjectileData {
    std::string name;
    SpriteData white_sprite;
    SpriteData black_sprite;
    float radius;
    float damage;
    float speed;

    bool Load(nlohmann::json json, SpriteDatabase const& spriteDatabase);
};

class ProjectileDatabase {
public:
    static std::unique_ptr<ProjectileDatabase> Load(std::filesystem::path const& path,
                                                    SpriteDatabase const& spriteDatabase);

    ProjectileData const* GetProjectileData(std::string const& name) const;

private:
    ProjectileDatabase(SpriteDatabase const& spriteDatabase)
        : m_spriteDatabase(spriteDatabase) {}

    std::map<std::string, ProjectileData> m_database;
    SpriteDatabase const& m_spriteDatabase;
};
