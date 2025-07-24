#pragma once

#include <canyon/graphics/surface_context.h>
#include <filesystem>
#include "gamedata_database.h"
#include "gamedata_fwd.h"

enum class GameDataCategory {
    Unknown = -1,
    Sprites,
    Projectiles,
    Weapons,
    Enemies,
    Spawners,
    Levels,
    Pickups,
};

struct SerializeContext {
    GameData const& gamedata;
    canyon::graphics::SurfaceContext& surfaceContext;
};

class GameData {
public:
    GameData();
    ~GameData();

    void LoadDirectory(std::filesystem::path const& directory, SerializeContext const& context);
    bool Load(std::filesystem::path const& path, SerializeContext const& context);

    template<typename T>
    T const* Get(std::string const& name) const {
        if constexpr (T::Category == GameDataCategory::Sprites) {
            return m_spriteDatabase.Get(name);
        } else if constexpr (T::Category == GameDataCategory::Projectiles) {
            return m_projectileDatabase.Get(name);
        } else if constexpr (T::Category == GameDataCategory::Weapons) {
            return m_weaponDatabase.Get(name);
        } else if constexpr (T::Category == GameDataCategory::Enemies) {
            return m_enemyDatabase.Get(name);
        } else if constexpr (T::Category == GameDataCategory::Spawners) {
            return m_spawnerDatabase.Get(name);
        } else if constexpr (T::Category == GameDataCategory::Levels) {
            return m_levelDatabase.Get(name);
        } else if constexpr (T::Category == GameDataCategory::Pickups) {
            return m_pickupDatabase.Get(name);
        } else {
            return nullptr;
        }
    }

    GameData(const GameData&) = delete;
    GameData(GameData&&) = delete;
    GameData& operator=(const GameData&) = delete;
    GameData& operator=(GameData&&) = delete;

    static SerializeContext const* s_currentContext;

private:
    Database<SpriteData> m_spriteDatabase;
    Database<ProjectileData> m_projectileDatabase;
    Database<WeaponData> m_weaponDatabase;
    Database<EnemyData> m_enemyDatabase;
    Database<SpawnerData> m_spawnerDatabase;
    Database<LevelData> m_levelDatabase;
    Database<PickupData> m_pickupDatabase;
};

