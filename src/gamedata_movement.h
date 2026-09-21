#pragma once

#include "utils_serialization.h"

// Tuning for an entity that steers itself with input. The velocity closes on the
// speed the input asks for rather than being set outright, so an entity is given
// weight by lowering the rates.
struct MovementData {
    float max_speed = 400.0f;
    // Units per second squared. Zero or less reaches max_speed at once, which is
    // movement with no weight to it at all.
    float acceleration = 0.0f;
    // The rate used once the input is released. Defaults to acceleration, so a
    // ship only drifts when it is asked to.
    float deceleration = 0.0f;
};

inline void from_json(nlohmann::json const& json, MovementData& data) {
    DATA_OPTIONAL(json, data, max_speed);
    DATA_OPTIONAL(json, data, acceleration);
    data.deceleration = data.acceleration;
    DATA_OPTIONAL(json, data, deceleration);
}
