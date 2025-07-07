#pragma once

#include "sprite_database.h"
#include <canyon/platform/window.h>
#include <nlohmann/json_fwd.hpp>

struct EnemyData {
    std::string name;
    std::string sprite_name;
    float health;
    float radius;
    uint32_t lifetime;
    std::string weapon_name;

    bool Load(nlohmann::json const& json);
};

class EnemyDatabase {
    public:
        static std::unique_ptr<EnemyDatabase> Load(std::filesystem::path const& path, SpriteDatabase const& spriteDatabase);
        EnemyData const* GetEnemyData(std::string const& name) const;

    private:
        EnemyDatabase(SpriteDatabase const& spriteDatabase)
            :m_spriteDatabase(spriteDatabase) {}

        std::map<std::string, EnemyData> m_database;
        SpriteDatabase const& m_spriteDatabase;
};
