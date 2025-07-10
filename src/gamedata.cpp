#include "gamedata.h"
#include "gamedata_level.h"

Gamedata::Gamedata(std::filesystem::path const& rootPath, canyon::graphics::SurfaceContext& surfaceContext) {
    SerializeContext context { *this, surfaceContext };
    m_spriteDatabase = Database<SpriteData>::Load(rootPath / "sprite_database.json", context);
    m_projectileDatabase = Database<ProjectileData>::Load(rootPath / "projectile_database.json", context);
    m_weaponDatabase = Database<WeaponData>::Load(rootPath / "weapon_database.json", context);
    m_enemyDatabase = Database<EnemyData>::Load(rootPath / "enemy_database.json", context);
    m_spawnerDatabase = Database<SpawnerData>::Load(rootPath / "spawner_database.json", context);
    m_levelDatabase = Database<LevelData>::Load(rootPath / "level_database.json", context);
}

