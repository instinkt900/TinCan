#pragma once

#include "enemy_database.h"
#include "projectile_database.h"
#include "spawner_database.h"
#include "sprite_database.h"
#include "weapon_database.h"
#include <memory>

class Databases {
public:
    Databases(std::filesystem::path const& rootPath, canyon::graphics::SurfaceContext& surfaceContext);

    SpriteDatabase const& GetSpriteDatabase() const { return *m_spriteDatabase; }
    ProjectileDatabase const& GetProjectileDatabase() const { return *m_projectileDatabase; }
    WeaponDatabase const& GetWeaponDatabase() const { return *m_weaponDatabase; }
    EnemyDatabase const& GetEnemyDatabase() const { return *m_enemyDatabase; }
    SpawnerDatabase const& GetSpawnerDatabase() const { return *m_spawnerDatabase; }

private:
    std::unique_ptr<SpriteDatabase> m_spriteDatabase;
    std::unique_ptr<ProjectileDatabase> m_projectileDatabase;
    std::unique_ptr<WeaponDatabase> m_weaponDatabase;
    std::unique_ptr<EnemyDatabase> m_enemyDatabase;
    std::unique_ptr<SpawnerDatabase> m_spawnerDatabase;
};
