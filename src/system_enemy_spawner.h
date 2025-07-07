#pragma once

#include "enemy_database.h"
#include "sprite_database.h"
#include "weapon_database.h"
#include <canyon/graphics/iimage.h>
#include <entt/entity/fwd.hpp>
#include <moth_ui/utils/vector.h>
#include <moth_ui/utils/rect.h>

struct ComponentEnemySpawner {
    bool m_active = false;
    int32_t m_cooldown = 0;
    int32_t m_maxCooldown = 1000;
    int32_t m_burst = 0;
    int32_t m_maxBurst = 1;
    int32_t m_burstDelay = 0;
    int32_t m_maxBurstDelay = 0;
    moth_ui::IntRect m_spawnRegion;
    std::string m_enemyName;
};

class SystemEnemySpawner {
public:
    static void Update(entt::registry& registry, uint32_t ticks, SpriteDatabase const& spriteDatabase,
                       EnemyDatabase const& enemyDatabase, WeaponDatabase const& weaponDatabase);
};
