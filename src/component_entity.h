#pragma once

#include <canyon/utils/vector.h>
#include <entt/entity/fwd.hpp>
#include <set>
#include "gamedata_entity.h"

struct ComponentEntity {
    Team m_team = Team::None;
    Affinity m_affinity = Affinity::None;
    float m_angle = 0;
    std::set<entt::entity> m_children;

    // cached values from the movement system
    canyon::FloatVec2 m_heading; // the direction we are facing
};

