#pragma once

#include "databases.h"
#include "enemy_database.h"
#include "sprite_database.h"
#include "system_behaviour.h"
#include "weapon_database.h"
#include <canyon/graphics/iimage.h>
#include <entt/entity/fwd.hpp>
#include <moth_ui/utils/vector.h>
#include <moth_ui/utils/rect.h>

struct ComponentEnemySpawner {
    bool m_active = false;
    int32_t m_count = 0;
    int32_t m_cooldown = 0;
    int32_t m_groupCount = 0;
    int32_t m_groupCooldown = 0;
    int32_t m_maxCooldown = 1000;
    int32_t m_maxGroupCount = 1;
    int32_t m_maxGroupCooldown = 0;
    std::string m_enemyName;
    std::string m_behaviourName;
    BehaviourParameterList m_behaviourParameters;
};

class SystemEnemySpawner {
public:
    static void Update(entt::registry& registry, uint32_t ticks, Databases& databases);
};
