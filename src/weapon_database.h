#pragma once

#include "sprite_database.h"
#include <canyon/platform/window.h>
#include <nlohmann/json_fwd.hpp>

struct BarrelData {
    moth_ui::IntVec2 offset;
    int32_t group;
};

struct WeaponData {
    std::string name;
    int32_t cooldown;
    bool player_tracking;
    std::string projectile_name;
    std::vector<BarrelData> barrels;

    bool Load(nlohmann::json const& json);
};

class WeaponDatabase {
    public:
        static std::unique_ptr<WeaponDatabase> Load(std::filesystem::path const& path, SpriteDatabase const& spriteDatabase);
        WeaponData const* GetWeaponData(std::string const& name) const;

    private:
        WeaponDatabase(SpriteDatabase const& spriteDatabase)
            :m_spriteDatabase(spriteDatabase) {}

        std::map<std::string, WeaponData> m_database;
        SpriteDatabase const& m_spriteDatabase;
};
