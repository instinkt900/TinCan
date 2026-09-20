#pragma once

#include "gamedata_behaviour.h"
#include <moth/graphics/utils/math_utils.h>
#include <moth/graphics/utils/vector.h>
#include <optional>

// The point a behaviour's paths are measured around: the first waypoint of its first path state. It is
// behaviour-wide rather than per-state, because rotating each state about its own start would tear the
// behaviour apart at every state boundary. A behaviour with no path state has no anchor.
inline std::optional<moth::gfx::FloatVec2> BehaviourAnchor(BehaviourData const& data) {
    for (auto const& state : data.states) {
        if (state.type == BehaviourType::Path && !state.waypoints.empty()) {
            return state.waypoints.front();
        }
    }
    return std::nullopt;
}

// Rotates every path in a behaviour about its anchor, clockwise on screen by an angle in degrees, and for
// a spawn-relative behaviour moves the anchor onto the origin as well, so that the first waypoint of the
// first path state lands exactly where the entity spawned once ComponentPathCache adds m_origin back.
// Target states keep their absolute world point, so they are neither rotated nor moved.
//
// This runs on the per-entity copy of the behaviour at spawn time. Everything the sampler derives is built
// from the waypoints, so transforming them carries the arc-length table, the Wave modifier's normal and
// the entry approach with it.
inline void TransformBehaviour(BehaviourData& data, float rotationDegrees) {
    auto const anchor = BehaviourAnchor(data);
    if (!anchor.has_value()) {
        return;
    }

    auto const radians = moth::gfx::DegToRad(rotationDegrees);
    auto const translation = data.space == BehaviourSpace::SpawnRelative
                                 ? -anchor.value()
                                 : moth::gfx::FloatVec2{ 0, 0 };

    for (auto& state : data.states) {
        if (state.type != BehaviourType::Path) {
            continue;
        }
        for (auto& waypoint : state.waypoints) {
            waypoint = anchor.value() + moth::gfx::Rotate2D(waypoint - anchor.value(), radians) + translation;
        }
    }
}

// What a spawn changes about the behaviour it uses. A typed struct rather than a map of named values: each
// field is known here, so the places that set one can name it, and nothing has to be resolved by string.
struct BehaviourOverride {
    // unset leaves every state at the speed it was authored with
    std::optional<float> speed;
    // degrees, clockwise on screen, turned about the behaviour's anchor
    float rotation{ 0.0f };
};

// Applies a spawn's override to its own copy of a behaviour.
inline void ApplyBehaviourOverride(BehaviourData& data, BehaviourOverride const& spawnOverride) {
    if (spawnOverride.speed.has_value()) {
        for (auto& state : data.states) {
            state.speed = spawnOverride.speed.value();
        }
    }
    TransformBehaviour(data, spawnOverride.rotation);
}
