#pragma once

#include <moth/graphics/graphics/surface_context.h>
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
    Pickups,
    Behaviours,
};

struct SerializeContext {
    GameData const& gamedata;
    moth::gfx::SurfaceContext& surfaceContext;
};

class GameData {
public:
    GameData();
    ~GameData();

    void LoadDirectory(std::filesystem::path const& directory, SerializeContext const& context);
    bool Load(std::filesystem::path const& path, SerializeContext const& context);

    template<typename T>
    Database<T> const* Get() const {
        if constexpr (T::Category == GameDataCategory::Sprites) {
            return &m_spriteDatabase;
        } else if constexpr (T::Category == GameDataCategory::Projectiles) {
            return &m_projectileDatabase;
        } else if constexpr (T::Category == GameDataCategory::Weapons) {
            return &m_weaponDatabase;
        } else if constexpr (T::Category == GameDataCategory::Enemies) {
            return &m_enemyDatabase;
        } else if constexpr (T::Category == GameDataCategory::Spawners) {
            return &m_spawnerDatabase;
        } else if constexpr (T::Category == GameDataCategory::Pickups) {
            return &m_pickupDatabase;
        } else if constexpr (T::Category == GameDataCategory::Behaviours) {
            return &m_behaviourDatabase;
        } else {
            return nullptr;
        }
    }

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
        } else if constexpr (T::Category == GameDataCategory::Pickups) {
            return m_pickupDatabase.Get(name);
        } else if constexpr (T::Category == GameDataCategory::Behaviours) {
            return m_behaviourDatabase.Get(name);
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
    Database<PickupData> m_pickupDatabase;
    Database<BehaviourData> m_behaviourDatabase;
};

