#pragma once

#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <functional>
#include <moth_ui/utils/vector.h>

enum Team {
    NONE = 0,
    PLAYER = 1,
    ENEMY = 2,
};

enum EnergyColor {
    WHITE,
    BLACK,
};

struct ComponentProjectile {
    entt::entity m_owner = entt::null;
    EnergyColor m_color = EnergyColor::WHITE;
    int32_t m_damage = 1;
};

struct ComponentHealth {
    int32_t m_currentHealth = 100;
    int32_t m_maxHealth = 100;
    bool m_dead = false;
    entt::entity m_lastDamager;
    std::function<void(entt::entity, entt::entity)> m_onDeath;
};

class SystemProjectile {
public:
    static void Update(entt::registry& registry, uint32_t ticks);
    static float Sweep(moth_ui::FloatVec2 const& stationary, moth_ui::FloatVec2 const& moving,
                        moth_ui::FloatVec2 const& velocity, float radius);

};
