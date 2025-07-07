#include "databases.h"

Databases::Databases(std::filesystem::path const& rootPath, canyon::graphics::SurfaceContext& surfaceContext) {
    m_spriteDatabase = SpriteDatabase::Load(rootPath / "sprite_database.json", surfaceContext);
    m_projectileDatabase = ProjectileDatabase::Load(rootPath / "projectile_database.json", *m_spriteDatabase);
    m_weaponDatabase = WeaponDatabase::Load(rootPath / "weapon_database.json", *m_spriteDatabase);
    m_enemyDatabase = EnemyDatabase::Load(rootPath / "enemy_database.json", *m_spriteDatabase);
    m_spawnerDatabase = SpawnerDatabase::Load(rootPath / "spawner_database.json");
}
