#pragma once

#include "gamedata_database.h"
#include "gamedata_pickup.h"
#include "gamedata_enemy.h"
#include "gamedata_level.h"
#include "gamedata_projectile.h"
#include "gamedata_spawner.h"
#include "gamedata_sprite.h"
#include "gamedata_weapon.h"

class GameData {
public:
    GameData() = default;

    Database<SpriteData>& GetSpriteDatabase() { return m_spriteDatabase; }
    Database<ProjectileData>& GetProjectileDatabase() { return m_projectileDatabase; }
    Database<WeaponData>& GetWeaponDatabase() { return m_weaponDatabase; }
    Database<EnemyData>& GetEnemyDatabase() { return m_enemyDatabase; }
    Database<SpawnerData>& GetSpawnerDatabase() { return m_spawnerDatabase; }
    Database<LevelData>& GetLevelDatabase() { return m_levelDatabase; }
    Database<PickupData>& GetPickupDatabase() { return m_pickupDatabase; }

    Database<SpriteData> const& GetSpriteDatabase() const { return m_spriteDatabase; }
    Database<ProjectileData> const& GetProjectileDatabase() const { return m_projectileDatabase; }
    Database<WeaponData> const& GetWeaponDatabase() const { return m_weaponDatabase; }
    Database<EnemyData> const& GetEnemyDatabase() const { return m_enemyDatabase; }
    Database<SpawnerData> const& GetSpawnerDatabase() const { return m_spawnerDatabase; }
    Database<LevelData> const& GetLevelDatabase() const { return m_levelDatabase; }
    Database<PickupData> const& GetPickupDatabase() const { return m_pickupDatabase; }
private:
    Database<SpriteData> m_spriteDatabase;
    Database<ProjectileData> m_projectileDatabase;
    Database<WeaponData> m_weaponDatabase;
    Database<EnemyData> m_enemyDatabase;
    Database<SpawnerData> m_spawnerDatabase;
    Database<LevelData> m_levelDatabase;
    Database<PickupData> m_pickupDatabase;
};
