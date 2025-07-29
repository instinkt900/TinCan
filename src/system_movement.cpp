#include "system_movement.h"
#include "component_entity.h"
#include "system_input.h"
#include <entt/entt.hpp>
#include "game_world.h"
#include "system_targeting.h"
#include "tags.h"
#include <canyon/utils/math_utils.h>

namespace {
    bool HasValidTarget(entt::registry& registry, ComponentTargeting const& targeting,
                        ComponentPosition const& position, canyon::FloatVec2& outToTarget) {
        if (targeting.m_currentTarget == entt::null || !registry.valid(targeting.m_currentTarget)) {
            return false;
        }
        auto const* targetPosition = registry.try_get<ComponentPosition>(targeting.m_currentTarget);
        if (targetPosition == nullptr) {
            return false;
        }
        auto const toTarget = targetPosition->m_position - position.m_position;
        if (canyon::LengthSq(toTarget) <= 0 || canyon::LengthSq(toTarget) <= 0) {
            return false;
        }

        outToTarget = toTarget;
        return true;
    }

    // void UpdateTargeting(GameWorld& world, uint32_t ticks) {
    //     auto const dt = static_cast<float>(ticks) / 1000.0f;
    //     auto& registry = world.GetRegistry();
    //     auto view = registry.view<ComponentTargeting, ComponentVelocity, ComponentPosition,
    //     ComponentEntity>(
    //         entt::exclude<DeadTag>);
    //     for (auto [entity, targeting, velocity, position, details] : view.each()) {
    //         canyon::FloatVec2 toTarget;
    //         bool const targetValid = HasValidTarget(registry, targeting, position, toTarget);
    //
    //         auto const maxTurnSpeed = targeting.m_maxTurnSpeed;
    //         auto const maxThrust = targeting.m_maxThrust;
    //         auto const maxSpeed = targeting.m_maxSpeed;
    //         auto const minDrag = targeting.m_minDrag;
    //         auto const maxDrag = targeting.m_maxDrag;
    //
    //         auto const heading = canyon::Rotate2D({ 0.0f, -1.0f }, details.m_angle);
    //
    //         canyon::FloatVec2 bearing;
    //         if (targetValid) {
    //             bearing = canyon::Normalized(toTarget);
    //             auto const angle = canyon::Angle(heading, bearing);
    //             auto const steering = std::clamp(angle, -maxTurnSpeed * dt, maxTurnSpeed * dt);
    //             details.m_angle += steering;
    //         }
    //
    //         float forwardThrust = maxThrust;
    //         if (targetValid) {
    //             auto const targetAlignment = canyon::Dot(heading, bearing);
    //             forwardThrust = std::abs(targetAlignment) * maxThrust;
    //         }
    //         auto const acceleration = heading * forwardThrust;
    //         velocity.m_velocity += acceleration * dt;
    //
    //         auto const course = canyon::Normalized(velocity.m_velocity);
    //         auto const alignment = std::max(0.0f, canyon::Dot(heading, course));
    //         auto const dragAmount = canyon::Lerp(minDrag, maxDrag, 1.0f - alignment);
    //         auto const drag = velocity.m_velocity * dragAmount;
    //         velocity.m_velocity -= drag * dt;
    //
    //         auto const speed = canyon::Length(velocity.m_velocity);
    //         if (speed >= maxSpeed) {
    //             velocity.m_velocity = (velocity.m_velocity / speed) * maxSpeed;
    //         }
    //     }
    // }

    void UpdateTargeting(GameWorld& world, uint32_t ticks) {
        auto const dt = static_cast<float>(ticks) / 1000.0f;
        auto& registry = world.GetRegistry();
        auto view = registry.view<ComponentTargeting, ComponentVelocity, ComponentPosition, ComponentEntity>(
            entt::exclude<DeadTag>);
        for (auto [entity, targeting, velocity, position, details] : view.each()) {
            canyon::FloatVec2 toTarget;
            bool const targetValid = HasValidTarget(registry, targeting, position, toTarget);

            // auto const maxTurnSpeed = static_cast<float>(M_PI) * 0.5f; // targeting.m_maxTurnSpeed;
            // auto const minThrust = targeting.m_maxThrust * 0.2f;
            // auto const maxThrust = targeting.m_maxThrust;
            // auto const maxSpeed = targeting.m_maxSpeed;
            // auto const minDrag = targeting.m_minDrag;
            // auto const maxDrag = targeting.m_maxDrag;

            auto const heading = canyon::Rotate2D({ 0.0f, -1.0f }, details.m_angle);

            if (targetValid) {
                auto const bearing = canyon::Normalized(toTarget);
                auto const angle = canyon::Angle(heading, bearing);
                auto const alignment = 1.0f - ((canyon::Dot(heading, bearing) + 1.0f) / 2.0f);
                auto const distance = 1.0f - std::min(1.0f, canyon::Length(toTarget) / 100.0f);
                auto const turnSpeed = canyon::Lerp(1.0f, 9.0f, std::max(alignment, distance)) * (angle > 0 ? 1.0f : -1.0f);
                details.m_angle += turnSpeed * dt;

                auto const speed = canyon::Length(velocity.m_velocity);
                velocity.m_velocity = canyon::Rotate2D({ 0.0f, -1.0f }, details.m_angle) * speed;
            }

            // auto const course = canyon::Normalized(velocity.m_velocity);
            // auto const alignment = std::max(0.0f, canyon::Dot(heading, course));
            // auto const dragAmount = canyon::Lerp(minDrag, maxDrag, 1.0f - alignment);
            // auto const drag = velocity.m_velocity * dragAmount;
            // velocity.m_velocity -= drag * dt;

            // auto const speed = canyon::Length(velocity.m_velocity);
            // if (speed >= maxSpeed) {
            //     velocity.m_velocity = (velocity.m_velocity / speed) * maxSpeed;
            // }
        }
    }

    void UpdatePosition(GameWorld& world, uint32_t ticks) {
        // update positions based on velocity
        auto const dt = static_cast<float>(ticks) / 1000.0f;
        auto& registry = world.GetRegistry();
        auto view = registry.view<ComponentPosition, ComponentVelocity>(entt::exclude<DeadTag, ComponentParenting>);
        for (auto [entity, pos, vel] : view.each()) {
            auto velocity = vel.m_velocity;
            if (auto* input = registry.try_get<ComponentInput>(entity)) {
                if (input->m_state[moth_ui::Key::Up]) {
                    velocity.y -= 400.0f;
                }
                if (input->m_state[moth_ui::Key::Down]) {
                    velocity.y += 400.0f;
                }
                if (input->m_state[moth_ui::Key::Left]) {
                    velocity.x -= 400.0f;
                }
                if (input->m_state[moth_ui::Key::Right]) {
                    velocity.x += 400.0f;
                }
            }

            pos.m_lastPosition = pos.m_position;
            pos.m_position += velocity * dt;
        }
    }

    void UpdateParenting(GameWorld& world, uint32_t ticks) {
        auto& registry = world.GetRegistry();
        auto view = registry.view<ComponentParenting, ComponentPosition>(entt::exclude<DeadTag>);
        for (auto [entity, parenting, position] : view.each()) {
            if (!registry.valid(parenting.m_parent)) {
                // parent is gone. kill any children
                registry.emplace<DeadTag>(entity);
                continue;
            }
            auto const* parentPosition = registry.try_get<ComponentPosition>(parenting.m_parent);
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
