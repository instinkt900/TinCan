#pragma once

#include "gamedata_database.h"
#include "gamedata_enemy.h"
#include "gamedata_level.h"
#include "gamedata_projectile.h"
#include "gamedata_spawner.h"
#include "gamedata_sprite.h"
#include "gamedata_weapon.h"

class Gamedata {
public:
    Gamedata(std::filesystem::path const& rootPath, canyon::graphics::SurfaceContext& surfaceContext);

    Database<SpriteData> const* GetSpriteDatabase() const { return m_spriteDatabase.get(); }
    Database<ProjectileData> const* GetProjectileDatabase() const { return m_projectileDatabase.get(); }
    Database<WeaponData> const* GetWeaponDatabase() const { return m_weaponDatabase.get(); }
    Database<EnemyData> const* GetEnemyDatabase() const { return m_enemyDatabase.get(); }
    Database<SpawnerData> const* GetSpawnerDatabase() const { return m_spawnerDatabase.get(); }
    Database<LevelData> const* GetLevelDatabase() const { return m_levelDatabase.get(); }

private:
    std::unique_ptr<Database<SpriteData>> m_spriteDatabase;
    std::unique_ptr<Database<ProjectileData>> m_projectileDatabase;
    std::unique_ptr<Database<WeaponData>> m_weaponDatabase;
    std::unique_ptr<Database<EnemyData>> m_enemyDatabase;
    std::unique_ptr<Database<SpawnerData>> m_spawnerDatabase;
    std::unique_ptr<Database<LevelData>> m_levelDatabase;
};
