#include "system_movement.h"
#include "component_entity.h"
#include "system_drawable.h"
#include "system_input.h"
#include <moth/ecs/world.h>
#include "game_world.h"
#include "system_targeting.h"
#include "tags.h"
#include <moth/graphics/utils/math_utils.h>

namespace {
    bool HasValidTarget(moth::ecs::World& registry, ComponentTargeting const& targeting,
                        ComponentPosition const& position, moth::gfx::FloatVec2& outToTarget) {
        if (targeting.m_currentTarget == moth::ecs::kNullEntity || !registry.Valid(targeting.m_currentTarget)) {
            return false;
        }
        auto const* targetPosition = registry.TryGet<ComponentPosition>(targeting.m_currentTarget);
        if (targetPosition == nullptr) {
            return false;
        }
        auto const toTarget = targetPosition->m_position - position.m_position;
        if (moth::gfx::LengthSq(toTarget) <= 0 || moth::gfx::LengthSq(toTarget) <= 0) {
            return false;
        }

        outToTarget = toTarget;
        return true;
    }

    // void UpdateTargeting(GameWorld& world, uint32_t ticks) {
    //     auto const dt = static_cast<float>(ticks) / 1000.0f;
    //     auto& registry = world.GetRegistry();
    //     auto view = registry.View<ComponentTargeting, ComponentVelocity, ComponentPosition,
    //     ComponentEntity>(
    //         moth::ecs::Exclude<DeadTag>);
    //     for (auto [entity, targeting, velocity, position, details] : view.each()) {
    //         moth::gfx::FloatVec2 toTarget;
    //         bool const targetValid = HasValidTarget(registry, targeting, position, toTarget);
    //
    //         auto const maxTurnSpeed = targeting.m_maxTurnSpeed;
    //         auto const maxThrust = targeting.m_maxThrust;
    //         auto const maxSpeed = targeting.m_maxSpeed;
    //         auto const minDrag = targeting.m_minDrag;
    //         auto const maxDrag = targeting.m_maxDrag;
    //
    //         auto const heading = moth::gfx::Rotate2D({ 0.0f, -1.0f }, details.m_angle);
    //
    //         moth::gfx::FloatVec2 bearing;
    //         if (targetValid) {
    //             bearing = moth::gfx::Normalized(toTarget);
    //             auto const angle = moth::gfx::Angle(heading, bearing);
    //             auto const steering = std::clamp(angle, -maxTurnSpeed * dt, maxTurnSpeed * dt);
    //             details.m_angle += steering;
    //         }
    //
    //         float forwardThrust = maxThrust;
    //         if (targetValid) {
    //             auto const targetAlignment = moth::gfx::Dot(heading, bearing);
    //             forwardThrust = std::abs(targetAlignment) * maxThrust;
    //         }
    //         auto const acceleration = heading * forwardThrust;
    //         velocity.m_velocity += acceleration * dt;
    //
    //         auto const course = moth::gfx::Normalized(velocity.m_velocity);
    //         auto const alignment = std::max(0.0f, moth::gfx::Dot(heading, course));
    //         auto const dragAmount = moth::gfx::Lerp(minDrag, maxDrag, 1.0f - alignment);
    //         auto const drag = velocity.m_velocity * dragAmount;
    //         velocity.m_velocity -= drag * dt;
    //
    //         auto const speed = moth::gfx::Length(velocity.m_velocity);
    //         if (speed >= maxSpeed) {
    //             velocity.m_velocity = (velocity.m_velocity / speed) * maxSpeed;
    //         }
    //     }
    // }

    void UpdateTargeting(GameWorld& world, uint32_t ticks) {
        auto const dt = static_cast<float>(ticks) / 1000.0f;
        auto& registry = world.GetRegistry();
        auto view = registry.View<ComponentTargeting, ComponentVelocity, ComponentPosition, ComponentEntity>(
            moth::ecs::Exclude<DeadTag>);
        for (auto [entity, targeting, velocity, position, details] : view.each()) {
            moth::gfx::FloatVec2 toTarget;
            bool const targetValid = HasValidTarget(registry, targeting, position, toTarget);

            // auto const maxTurnSpeed = static_cast<float>(M_PI) * 0.5f; // targeting.m_maxTurnSpeed;
            // auto const minThrust = targeting.m_maxThrust * 0.2f;
            // auto const maxThrust = targeting.m_maxThrust;
            // auto const maxSpeed = targeting.m_maxSpeed;
            // auto const minDrag = targeting.m_minDrag;
            // auto const maxDrag = targeting.m_maxDrag;

            auto const heading = moth::gfx::Rotate2D({ 0.0f, -1.0f }, details.m_angle);

            if (targetValid) {
                auto const bearing = moth::gfx::Normalized(toTarget);
                auto const angle = moth::gfx::Angle(heading, bearing);
                auto const alignment = 1.0f - ((moth::gfx::Dot(heading, bearing) + 1.0f) / 2.0f);
                auto const distance = 1.0f - std::min(1.0f, moth::gfx::Length(toTarget) / 100.0f);
                auto const turnSpeed = moth::gfx::Lerp(1.0f, 9.0f, std::max(alignment, distance)) * (angle > 0 ? 1.0f : -1.0f);
                details.m_angle += turnSpeed * dt;

                auto const speed = moth::gfx::Length(velocity.m_velocity);
                velocity.m_velocity = moth::gfx::Rotate2D({ 0.0f, -1.0f }, details.m_angle) * speed;
            }

            // auto const course = moth::gfx::Normalized(velocity.m_velocity);
            // auto const alignment = std::max(0.0f, moth::gfx::Dot(heading, course));
            // auto const dragAmount = moth::gfx::Lerp(minDrag, maxDrag, 1.0f - alignment);
            // auto const drag = velocity.m_velocity * dragAmount;
            // velocity.m_velocity -= drag * dt;

            // auto const speed = moth::gfx::Length(velocity.m_velocity);
            // if (speed >= maxSpeed) {
            //     velocity.m_velocity = (velocity.m_velocity / speed) * maxSpeed;
            // }
        }
    }

    // The velocity the input is asking for, closed on at the rate the entity's
    // tuning allows. An acceleration of zero or less arrives at once, which is
    // the weightless movement this started out with.
    moth::gfx::FloatVec2 SteerVelocity(ComponentInput const& input, ComponentMovement const& movement,
                                       moth::gfx::FloatVec2 const& velocity, float dt) {
        moth::gfx::FloatVec2 direction{ 0.0f, 0.0f };
        if (input.Held(InputAction::MoveUp)) {
            direction.y -= 1.0f;
        }
        if (input.Held(InputAction::MoveDown)) {
            direction.y += 1.0f;
        }
        if (input.Held(InputAction::MoveLeft)) {
            direction.x -= 1.0f;
        }
        if (input.Held(InputAction::MoveRight)) {
            direction.x += 1.0f;
        }

        bool const steering = moth::gfx::LengthSq(direction) > 0;
        auto const target = steering ? moth::gfx::Normalized(direction) * movement.m_maxSpeed
                                     : moth::gfx::FloatVec2{ 0.0f, 0.0f };
        auto const rate = steering ? movement.m_acceleration : movement.m_deceleration;
        if (rate <= 0.0f) {
            return target;
        }

        // Turning back on an axis drops the speed already held that way instead
        // of braking through it, so weight is felt getting up to speed but a
        // reversal stays sharp. A product below zero is the sign change, and an
        // axis with no input is left for the deceleration rate to bring down.
        auto current = velocity;
        if (direction.x * current.x < 0.0f) {
            current.x = 0.0f;
        }
        if (direction.y * current.y < 0.0f) {
            current.y = 0.0f;
        }

        // Stepping by distance rather than by axis keeps the closing rate the
        // same in every direction, and stopping at the target avoids overshoot
        // at low frame rates.
        auto const delta = target - current;
        auto const distance = moth::gfx::Length(delta);
        auto const step = rate * dt;
        if (step >= distance) {
            return target;
        }
        return current + (delta / distance) * step;
    }

    void UpdatePosition(GameWorld& world, uint32_t ticks) {
        // update positions based on velocity
        auto const dt = static_cast<float>(ticks) / 1000.0f;
        auto& registry = world.GetRegistry();
        auto view = registry.View<ComponentPosition, ComponentVelocity>(moth::ecs::Exclude<DeadTag, ComponentParenting>);
        for (auto [entity, pos, vel] : view.each()) {
            if (auto const* input = registry.TryGet<ComponentInput>(entity)) {
                // An input entity carrying no tuning of its own keeps the
                // defaults, which move it the way it moved before.
                static ComponentMovement const defaultMovement;
                auto const* movement = registry.TryGet<ComponentMovement>(entity);
                vel.m_velocity = SteerVelocity(*input, movement != nullptr ? *movement : defaultMovement,
                                               vel.m_velocity, dt);
            }
            auto const velocity = vel.m_velocity;

            if (auto* sprite = registry.TryGet<ComponentSprite>(entity)) {
                if (velocity.x < -10.0f) {
                    SetSpriteState(*sprite, "turn_left");
                } else if (velocity.x > 10.0f) {
                    SetSpriteState(*sprite, "turn_right");
                } else {
                    SetSpriteState(*sprite, "idle");
                }
            }

            pos.m_lastPosition = pos.m_position;
            pos.m_position += velocity * dt;
        }
    }

    void UpdateParenting(GameWorld& world, uint32_t ticks) {
        auto& registry = world.GetRegistry();
        auto view = registry.View<ComponentParenting, ComponentPosition>(moth::ecs::Exclude<DeadTag>);
        for (auto [entity, parenting, position] : view.each()) {
            if (!registry.Valid(parenting.m_parent)) {
                // parent is gone. kill any children
                registry.Emplace<DeadTag>(entity);
                continue;
            }
            auto const* parentPosition = registry.TryGet<ComponentPosition>(parenting.m_parent);
            if (parentPosition != nullptr) {
                position.m_lastPosition = position.m_position;
                position.m_position = parentPosition->m_position + parenting.m_offset;
            }
        }
    }
}

void SystemMovement::Update(GameWorld& world, uint32_t ticks) {
    UpdateTargeting(world, ticks);
    UpdatePosition(world, ticks);
    UpdateParenting(world, ticks);
}
