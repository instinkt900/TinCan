#include "system_behaviour.h"
#include "game_world.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "tags.h"
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

// namespace {
//     template <typename T>
//     T GetParameter(BehaviourParameterList const& parameterList, std::string const& name,
//                    T const defaultValue) {
//         auto entry = parameterList.find(name);
//         if (entry == parameterList.end()) {
//             return defaultValue;
//         }
//         return std::get<T>(entry->second);
//     }
// }
//
// void BehaviourStraight(moth::ecs::World& registry, moth::ecs::Entity entity, ComponentBehaviour& behaviour,
//                        ComponentPosition& position) {
//     float const speed = behaviour.m_speed;
//     float const t = static_cast<float>(behaviour.m_ticks) / 1000.0f;
//     auto const newPosition = behaviour.m_offset + moth::gfx::FloatVec2{ 0, t * speed };
//     position.m_lastPosition = position.m_position;
//     position.m_position = newPosition;
// }
//
// void BehaviourWave(moth::ecs::World& registry, moth::ecs::Entity entity, ComponentBehaviour& behaviour,
//                    ComponentPosition& position) {
//     float const freq = GetParameter(behaviour.m_parameters, "frequency", 1.0f);
//     float const amp = GetParameter(behaviour.m_parameters, "amplitude", 1.0f);
//     float const speed = behaviour.m_speed;
//
//     auto* cacheData = registry.TryGet<ComponentCurveCache>(entity);
//     if (cacheData == nullptr) {
//         cacheData = &registry.Emplace<ComponentCurveCache>(entity);
//
//         float const MaxT = 1500.0f;
//         float const TStep = 50.0f;
//
//         CurveSample lastSample;
//         for (float t = 0; t < MaxT; t += TStep) {
//             float const tf = t / 1000.0f;
//             auto& sample = cacheData->samples.emplace_back();
//             sample.position.x = std::sin(tf * freq) * amp;
//             sample.position.y = t;
//             if (t == 0) {
//                 sample.segment_length = 0;
//                 sample.total_distance = 0;
//             } else {
//                 sample.segment_length = moth::gfx::Distance(sample.position, lastSample.position);
//                 sample.total_distance = lastSample.total_distance + sample.segment_length;
//             }
//             lastSample = sample;
//         }
//         cacheData->total_length = lastSample.total_distance;
//     }
//
//     auto const distanceTravelled = speed * (static_cast<float>(behaviour.m_ticks) / 1000.0f);
//     size_t index = 0;
//     for (size_t i = 1; i < cacheData->samples.size(); ++i) {
//         if (distanceTravelled < cacheData->samples[i].total_distance) {
//             break;
//         }
//         index = i;
//     }
//     auto const& pastNode = cacheData->samples[index];
//
//     position.m_lastPosition = position.m_position;
//     position.m_position = pastNode.position;
//
//     if (index < (cacheData->samples.size() - 1)) {
//         auto const& nextNode = cacheData->samples[index + 1];
//         float const remainder = distanceTravelled - pastNode.total_distance;
//         if (remainder > 0) {
//             float const factor = remainder / nextNode.segment_length;
//             auto const delta = nextNode.position - pastNode.position;
//             position.m_position += delta * factor;
//         }
//     }
// }
//
// void BehaviourSpline(moth::ecs::World& registry, moth::ecs::Entity entity, ComponentBehaviour& behaviour,
//                      ComponentPosition& position) {
//
//     auto* cacheData = registry.TryGet<ComponentSplineCache>(entity);
//     if (cacheData == nullptr) {
//         cacheData = &registry.Emplace<ComponentSplineCache>(entity);
//         cacheData->m_lut = BuildArcLengthTable(behaviour.m_spline);
//     }
//
//     float const speed = behaviour.m_speed;
//     auto const distanceTravelled = speed * (static_cast<float>(behaviour.m_ticks) / 1000.0f);
//     auto const sample = SamplePathByDistance(behaviour.m_spline, cacheData->m_lut, distanceTravelled);
//
//     position.m_lastPosition = position.m_position;
//     position.m_position = sample;
// }

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

    // the direction of travel at this point on the path, found from the points either side of it
    moth::gfx::FloatVec2 PathTangent(BehaviourStateData const& state, float u) {
        float const step = 0.001f;
        auto const ahead = SamplePath(state.waypoints, state.interpolation, std::min(1.0f, u + step));
        auto const behind = SamplePath(state.waypoints, state.interpolation, std::max(0.0f, u - step));
        auto const delta = ahead - behind;
        if (moth::gfx::LengthSq(delta) <= 0.0f) {
            return { 0, 0 };
        }
        return moth::gfx::Normalized(delta);
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

    auto position = SamplePathByDistance(state.waypoints, state.interpolation, lut, u * totalLength);

    for (auto const& modifier : state.modifiers) {
        if (modifier.type != ModifierType::Wave) {
            continue;
        }
        auto const tangent = PathTangent(state, u);
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
        auto& position = target.positionComponent;
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
                                      ? position.m_position
                                      : moth::gfx::FloatVec2{ 0, 0 };
            cacheData->m_lut = BuildArcLengthTable(state.waypoints, state.interpolation);
        }
    }

    void Deinit(BehaviourTarget const& target) override {
        target.registry.Remove<ComponentPathCache>(target.entity);
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
        if (behaviour.m_stateIdx < behaviour.m_data.states.size()) {
            BehaviourTarget const target{ registry, entity, behaviour, position, velocity };
            auto pendingTicks = ticks;
            while (pendingTicks > 0) {
                if (behaviour.m_currentState != nullptr) {
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

                // enter the next state
                if (behaviour.m_stateIdx >= behaviour.m_data.states.size()) {
                    break;
                }
                behaviour.m_currentState = &behaviour.m_data.states[behaviour.m_stateIdx];
                if (auto* handler = FindHandler(behaviour.m_currentState->type); handler != nullptr) {
                    handler->Init(target);
                }
            }
        }
    }
}
