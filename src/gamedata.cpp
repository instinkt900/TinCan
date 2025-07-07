#include "gamedata.h"

Gamedata::Gamedata(std::filesystem::path const& rootPath, canyon::graphics::SurfaceContext& surfaceContext) {
    m_spriteDatabase = Database<SpriteData>::Load(rootPath / "sprite_database.json", *this, surfaceContext);
    m_projectileDatabase = Database<ProjectileData>::Load(rootPath / "projectile_database.json", *this, surfaceContext);
    m_weaponDatabase = Database<WeaponData>::Load(rootPath / "weapon_database.json", *this, surfaceContext);
    m_enemyDatabase = Database<EnemyData>::Load(rootPath / "enemy_database.json", *this, surfaceContext);
    m_spawnerDatabase = Database<SpawnerData>::Load(rootPath / "spawner_database.json", *this, surfaceContext);
}

