#include "system_behaviour.h"
#include "component_entity.h"
#include "game_world.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "system_weapon.h"
#include "tags.h"
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <optional>

namespace {
    float ApplyEase(EaseType ease, float u) {
        switch (ease) {
        case EaseType::In:
            return u * u;
        case EaseType::Out:
            return 1.0f - ((1.0f - u) * (1.0f - u));
        case EaseType::InOut:
            return u * u * (3.0f - (2.0f * u));
        }
        return u;
    }

    // switches the weapons of an entity and of all its sections on or off. a weapon switched off finishes a
    // burst it has started, and its cooldown keeps running
    void SetWeaponsActive(moth::ecs::World& registry, moth::ecs::Entity entity, bool active) {
        auto const setActive = [&registry, active](moth::ecs::Entity target) {
            if (auto* weapons = registry.TryGet<ComponentWeapon>(target); weapons != nullptr) {
                for (auto& weapon : weapons->m_weapons) {
                    weapon.m_active = active;
                }
            }
        };
        setActive(entity);
        if (auto const* details = registry.TryGet<ComponentEntity>(entity); details != nullptr) {
            for (auto const child : details->m_children) {
                setActive(child);
            }
        }
    }
}

moth::gfx::FloatVec2 SamplePathState(BehaviourStateData const& state, ArcLengthLUT const& lut,
                                     float distanceTravelled, uint32_t phaseIndex) {
    auto const totalLength = lut.distances.back();
    auto u = totalLength > 0.0f ? std::clamp(distanceTravelled / totalLength, 0.0f, 1.0f) : 0.0f;

    float phase = 0.0f;
    for (auto const& modifier : state.modifiers) {
        if (modifier.type == ModifierType::Phase) {
            phase += modifier.cycles * static_cast<float>(phaseIndex);
        }
    }
    for (auto const& modifier : state.modifiers) {
        if (modifier.type == ModifierType::Ease) {
            u = ApplyEase(modifier.ease, u);
        }
    }

    auto const distanceAlong = u * totalLength;
    auto position = SamplePathByDistance(state.waypoints, state.interpolation, lut, distanceAlong);

    for (auto const& modifier : state.modifiers) {
        if (modifier.type != ModifierType::Wave) {
            continue;
        }
        auto const tangent =
            PathTangentAtDistance(state.waypoints, state.interpolation, lut, distanceAlong);
        auto const normal = moth::gfx::FloatVec2{ -tangent.y, tangent.x };
        auto const angle = 2.0f * static_cast<float>(M_PI) * ((modifier.frequency * u) + phase);
        position += normal * (modifier.amplitude * std::sin(angle));
    }

    return position;
}

struct BehaviourTarget {
    moth::ecs::World& registry;
    moth::ecs::Entity entity;
    ComponentBehaviour& behaviourComponent;
    ComponentPosition& positionComponent;
    ComponentVelocity& velocityComponent;
};

class BehaviourHandler {
public:
    virtual void Init(BehaviourTarget const& target) {}
    virtual void Deinit(BehaviourTarget const& target) {}
    virtual void Update(BehaviourTarget const& target, uint32_t ticks) {}
    // Runs before the state's own clock starts, for a state that has to travel to its start first.
    // Returns the ticks it did not use, which are the ones the state itself then gets. While it uses all
    // of them the state has not begun, so its duration is untouched.
    virtual uint32_t Enter(BehaviourTarget const& target, uint32_t ticks) { return ticks; }
    // asked only of a state with no duration, which then runs until its handler says it is done
    virtual bool IsFinished(BehaviourTarget const& target) const { return false; }

    BehaviourHandler() = default;
    virtual ~BehaviourHandler() = default;
    BehaviourHandler(BehaviourHandler const&) = default;
    BehaviourHandler(BehaviourHandler&&) = default;
    BehaviourHandler& operator=(BehaviourHandler const&) = default;
    BehaviourHandler& operator=(BehaviourHandler&&) = default;
};

class BehaviourHandlerHold : public BehaviourHandler {
public:
    void Init(BehaviourTarget const& target) override { target.velocityComponent.m_velocity = { 0, 0 }; }
};

class BehaviourHandlerPath : public BehaviourHandler {
public:
    void Init(BehaviourTarget const& target) override {
        auto& registry = target.registry;
        auto entity = target.entity;
        auto& behaviour = target.behaviourComponent;
        auto const& position = target.positionComponent;
        auto const& state = *behaviour.m_currentState;

        // a path drives position directly, so nothing is left for the movement system to apply
        target.velocityComponent.m_velocity = { 0, 0 };

        if (state.waypoints.size() < 2) {
            // a path needs two points before it goes anywhere. hold position for the state's duration
            // rather than refusing to spawn, because the editor builds a path a point at a time. Init runs
            // once per entry into a state, so this does not repeat every frame.
            spdlog::warn("Behaviour state {} '{}' has fewer than two waypoints, so it does not move",
                         behaviour.m_stateIdx, state.label);
            return;
        }

        auto* cacheData = registry.TryGet<ComponentPathCache>(entity);
        if (cacheData == nullptr) {
            cacheData = &registry.Emplace<ComponentPathCache>(entity);
            cacheData->m_offset = behaviour.m_data.space == BehaviourSpace::SpawnRelative
                                      ? behaviour.m_origin
                                      : moth::gfx::FloatVec2{ 0, 0 };
            cacheData->m_lut = BuildArcLengthTable(state.waypoints, state.interpolation);
            SetUpEntry(*cacheData, state, position);
        }
    }

    void Deinit(BehaviourTarget const& target) override {
        // leave the entity travelling the way the path was heading at its end, so that a behaviour whose
        // last state is a path carries on off the screen and is cleaned up by its bounds, rather than
        // stopping dead wherever the waypoints ran out. A state that follows this one sets its own
        // velocity in Init, so this only outlives the state when nothing comes after it. The direction is
        // the geometry's tangent, not the step just taken, which a Wave modifier would have swinging
        // across the path.
        if (auto const* cacheData = target.registry.TryGet<ComponentPathCache>(target.entity)) {
            auto const& state = *target.behaviourComponent.m_currentState;
            auto const tangent = PathTangentAtDistance(state.waypoints, state.interpolation, cacheData->m_lut,
                                                       cacheData->m_lut.distances.back());
            target.velocityComponent.m_velocity = tangent * state.speed;
        }
        target.registry.Remove<ComponentPathCache>(target.entity);
    }

    uint32_t Enter(BehaviourTarget const& target, uint32_t ticks) override {
        auto* cacheData = target.registry.TryGet<ComponentPathCache>(target.entity);
        if (cacheData == nullptr || cacheData->m_entryPoints.empty()) {
            return ticks;
        }

        auto& position = target.positionComponent;
        auto const totalLength = cacheData->m_entryLut.distances.back();
        auto const needed = totalLength / cacheData->m_entrySpeed;
        auto const elapsed = cacheData->m_entryTime + (static_cast<float>(ticks) / 1000.0f);

        if (elapsed >= needed) {
            // the approach finishes part way through this tick, and what is left over starts the state
            auto const spare = elapsed - needed;
            cacheData->m_entryPoints.clear();
            cacheData->m_entryLut = {};
            return static_cast<uint32_t>(spare * 1000.0f);
        }

        cacheData->m_entryTime = elapsed;
        position.m_lastPosition = position.m_position;
        position.m_position =
            SamplePathByDistance(cacheData->m_entryPoints, PathInterpolation::Linear, cacheData->m_entryLut,
                                 elapsed * cacheData->m_entrySpeed);
        return 0;
    }

    void Update(BehaviourTarget const& target, uint32_t ticks) override {
        auto& behaviour = target.behaviourComponent;
        auto& position = target.positionComponent;

        auto const* cacheData = target.registry.TryGet<ComponentPathCache>(target.entity);
        if (cacheData == nullptr) {
            // Init found a path with too few points and left no cache, so the entity holds position
            return;
        }

        auto const& state = *behaviour.m_currentState;
        auto const sample =
            SamplePathState(state, cacheData->m_lut, DistanceTravelled(target), behaviour.m_phaseIndex);

        position.m_lastPosition = position.m_position;
        position.m_position = cacheData->m_offset + sample;
    }

    bool IsFinished(BehaviourTarget const& target) const override {
        auto const* cacheData = target.registry.TryGet<ComponentPathCache>(target.entity);
        if (cacheData == nullptr) {
            return false;
        }
        return DistanceTravelled(target) >= cacheData->m_lut.distances.back();
    }

private:
    // Works out the approach into the start of the path, if the state asks for one. The direction the
    // entity is already travelling comes from the step it took last frame, so it does not matter what kind
    // of state it came from; at a spawn it is zero, and the approach then aims straight at the start.
    static void SetUpEntry(ComponentPathCache& cache, BehaviourStateData const& state,
                           ComponentPosition const& position) {
        auto const speed = state.entry.speed > 0.0f ? state.entry.speed : state.speed;
        if (state.entry.type == TransitionType::Snap || speed <= 0.0f) {
            return;
        }

        auto const start = cache.m_offset + SamplePath(state.waypoints, state.interpolation, 0.0f);
        if (moth::gfx::LengthSq(start - position.m_position) <= 0.0f) {
            return;
        }

        auto const travelled = position.m_position - position.m_lastPosition;
        auto const heading = moth::gfx::LengthSq(travelled) > 0.0f ? moth::gfx::Normalized(travelled)
                                                                   : moth::gfx::FloatVec2{ 0, 0 };
        cache.m_entryPoints = BuildTransitionPath(state.entry.type, position.m_position, heading, start,
                                                  PathTangentAtDistance(state.waypoints, state.interpolation, cache.m_lut, 0.0f));
        cache.m_entryLut = BuildArcLengthTable(cache.m_entryPoints, PathInterpolation::Linear);
        cache.m_entryTime = 0.0f;
        cache.m_entrySpeed = speed;
        if (cache.m_entryLut.distances.back() <= 0.0f) {
            cache.m_entryPoints.clear();
        }
    }

    static float DistanceTravelled(BehaviourTarget const& target) {
        auto const& behaviour = target.behaviourComponent;
        return behaviour.m_currentState->speed * (static_cast<float>(behaviour.m_stateTime) / 1000.0f);
    }
};

class BehaviourHandlerTarget : public BehaviourHandler {
public:
    void Init(BehaviourTarget const& target) override { Update(target, 0); }

    void Update(BehaviourTarget const& target, uint32_t ticks) override {
        moth::gfx::FloatVec2 targetPos;
        if (target.behaviourComponent.m_currentState->target == TargetType::Point) {
            targetPos = target.behaviourComponent.m_currentState->point;
        } else if (target.behaviourComponent.m_currentState->target == TargetType::Player) {
            auto view = target.registry.View<ComponentPosition, PlayerTag>();
            for (auto [entity, position] : view.each()) {
                targetPos = position.m_position;
            }
        }
        auto const toTarget = targetPos - target.positionComponent.m_position;
        if (moth::gfx::LengthSq(toTarget) <= 0.0f) {
            // already at the target, and a zero vector can't be normalized
            target.velocityComponent.m_velocity = { 0, 0 };
            return;
        }
        target.velocityComponent.m_velocity =
            moth::gfx::Normalized(toTarget) * target.behaviourComponent.m_currentState->speed;
    }
};

std::map<BehaviourType, std::shared_ptr<BehaviourHandler>> const BehaviourHandlers{
    { BehaviourType::Hold, std::make_shared<BehaviourHandlerHold>() },
    { BehaviourType::Path, std::make_shared<BehaviourHandlerPath>() },
    { BehaviourType::Target, std::make_shared<BehaviourHandlerTarget>() }
};

BehaviourHandler* FindHandler(BehaviourType type) {
    auto const it = BehaviourHandlers.find(type);
    return it != BehaviourHandlers.end() ? it->second.get() : nullptr;
}

void SystemBehaviour::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto view = registry.View<ComponentBehaviour, ComponentPosition, ComponentVelocity>();

    for (auto const& [entity, behaviour, position, velocity] : view.each()) {
        if (!behaviour.m_init) {
            // the behaviour starts here, which is what a spawn-relative one measures its states from.
            // taken on the first update rather than at spawn, so that it does not depend on whoever
            // created the entity remembering to set it
            behaviour.m_origin = position.m_position;
            behaviour.m_init = true;
        }

        if (behaviour.m_stateIdx < behaviour.m_data.states.size()) {
            BehaviourTarget const target{ registry, entity, behaviour, position, velocity };
            auto pendingTicks = ticks;
            // the ticks that were left when the loop state was entered during this update, if it was
            std::optional<uint32_t> loopEnteredTicks;
            while (pendingTicks > 0) {
                if (behaviour.m_currentState != nullptr) {
                    auto* entering = FindHandler(behaviour.m_currentState->type);
                    if (entering != nullptr) {
                        // travelling to the start of the state happens before its own clock starts
                        pendingTicks = entering->Enter(target, pendingTicks);
                        if (pendingTicks == 0) {
                            break;
                        }
                    }

                    auto const maxTime = behaviour.m_currentState->duration;
                    auto const usedTicks =
                        maxTime > 0 ? std::min(maxTime - behaviour.m_stateTime, pendingTicks) : pendingTicks;
                    behaviour.m_stateTime += usedTicks;
                    pendingTicks -= usedTicks;

                    auto* handler = FindHandler(behaviour.m_currentState->type);
                    if (handler != nullptr) {
                        handler->Update(target, usedTicks);
                    }

                    // a state with a duration ends when it runs out. one without runs until its handler
                    // says it is done, which for a path is when it reaches the end of its waypoints
                    bool const finished = maxTime > 0 ? behaviour.m_stateTime >= maxTime
                                                      : handler != nullptr && handler->IsFinished(target);
                    if (!finished) {
                        break;
                    }
                    if (handler != nullptr) {
                        handler->Deinit(target);
                    }
                    ++behaviour.m_stateIdx;
                    behaviour.m_currentState = nullptr;
                    behaviour.m_stateTime = 0;
                }

                // enter the next state, or go round again from the loop state after the last one
                if (behaviour.m_stateIdx >= behaviour.m_data.states.size()) {
                    auto const loopFrom = behaviour.m_data.loop_from;
                    if (loopFrom < 0 || loopFrom >= static_cast<int>(behaviour.m_data.states.size())) {
                        break;
                    }
                    // a spawn-relative behaviour moves its origin by how far the pass took the entity, so the
                    // next pass starts where this one ended rather than replaying in the same place
                    if (behaviour.m_data.space == BehaviourSpace::SpawnRelative) {
                        behaviour.m_origin += position.m_position - behaviour.m_loopStart;
                    }
                    behaviour.m_stateIdx = static_cast<uint32_t>(loopFrom);
                    // a pass that used no time would go round forever inside this update, so the next pass
                    // waits for the next update
                    if (loopEnteredTicks == pendingTicks) {
                        break;
                    }
                }
                behaviour.m_currentState = &behaviour.m_data.states[behaviour.m_stateIdx];
                if (static_cast<int>(behaviour.m_stateIdx) == behaviour.m_data.loop_from) {
                    behaviour.m_loopStart = position.m_position;
                    loopEnteredTicks = pendingTicks;
                }
                // each state says whether the entity fires while it runs. the last state's setting stays
                // after the behaviour ends
                SetWeaponsActive(registry, entity, behaviour.m_currentState->fire);
                if (auto* handler = FindHandler(behaviour.m_currentState->type); handler != nullptr) {
                    handler->Init(target);
                }
            }
        }
    }
}
