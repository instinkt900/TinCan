#include "system_behaviour.h"
#include "game_world.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include "tags.h"
#include <magic_enum.hpp>

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
// void BehaviourStraight(entt::registry& registry, entt::entity entity, ComponentBehaviour& behaviour,
//                        ComponentPosition& position) {
//     float const speed = behaviour.m_speed;
//     float const t = static_cast<float>(behaviour.m_ticks) / 1000.0f;
//     auto const newPosition = behaviour.m_offset + canyon::FloatVec2{ 0, t * speed };
//     position.m_lastPosition = position.m_position;
//     position.m_position = newPosition;
// }
//
// void BehaviourWave(entt::registry& registry, entt::entity entity, ComponentBehaviour& behaviour,
//                    ComponentPosition& position) {
//     float const freq = GetParameter(behaviour.m_parameters, "frequency", 1.0f);
//     float const amp = GetParameter(behaviour.m_parameters, "amplitude", 1.0f);
//     float const speed = behaviour.m_speed;
//
//     auto* cacheData = registry.try_get<ComponentCurveCache>(entity);
//     if (cacheData == nullptr) {
//         cacheData = &registry.emplace<ComponentCurveCache>(entity);
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
//                 sample.segment_length = canyon::Distance(sample.position, lastSample.position);
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
// void BehaviourSpline(entt::registry& registry, entt::entity entity, ComponentBehaviour& behaviour,
//                      ComponentPosition& position) {
//
//     auto* cacheData = registry.try_get<ComponentSplineCache>(entity);
//     if (cacheData == nullptr) {
//         cacheData = &registry.emplace<ComponentSplineCache>(entity);
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

struct BehaviourTarget {
    entt::registry& registry;
    entt::entity entity;
    ComponentBehaviour& behaviourComponent;
    ComponentPosition& positionComponent;
    ComponentVelocity& velocityComponent;
};

class BehaviourHandler {
public:
    virtual void Init(BehaviourTarget const& target) {}
    virtual void Deinit(BehaviourTarget const& target) {}
    virtual void Update(BehaviourTarget const& target, uint32_t ticks) {}

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

class BehaviourHandlerLine : public BehaviourHandler {
public:
    void Init(BehaviourTarget const& target) override {
        target.velocityComponent.m_velocity = target.behaviourComponent.m_currentState->velocity;
    }
};

class BehaviourHandlerTarget : public BehaviourHandler {
public:
    void Init(BehaviourTarget const& target) override { Update(target, 0); }

    void Update(BehaviourTarget const& target, uint32_t ticks) override {
        canyon::FloatVec2 targetPos;
        if (target.behaviourComponent.m_currentState->target == TargetType::Point) {
            targetPos = target.behaviourComponent.m_currentState->point;
        } else if (target.behaviourComponent.m_currentState->target == TargetType::Player) {
            auto view = target.registry.view<ComponentPosition, PlayerTag>();
            for (auto [entity, position] : view.each()) {
                targetPos = position.m_position;
            }
        }
        target.velocityComponent.m_velocity =
            canyon::Normalized(targetPos - target.positionComponent.m_position) *
            target.behaviourComponent.m_currentState->speed;
    }
};

class BehaviourHandlerWave : public BehaviourHandler {
public:
    void Init(BehaviourTarget const& target) override {
        auto& registry = target.registry;
        auto entity = target.entity;
        auto freq = target.behaviourComponent.m_currentState->frequency;
        auto amp = target.behaviourComponent.m_currentState->amplitude;
        auto const& position = target.positionComponent.m_position;

        target.velocityComponent.m_velocity = { 0, 0 };

        auto* cacheData = registry.try_get<ComponentCurveCache>(entity);
        if (cacheData == nullptr) {
            cacheData = &registry.emplace<ComponentCurveCache>(entity);

            float const MaxT = 1500.0f;
            float const TStep = 50.0f;

            CurveSample lastSample;
            for (float t = 0; t < MaxT; t += TStep) {
                float const tf = t / 1000.0f;
                auto& sample = cacheData->samples.emplace_back();
                sample.position.x = position.x + std::sin(tf * freq) * amp;
                sample.position.y = position.y + t;
                if (t == 0) {
                    sample.segment_length = 0;
                    sample.total_distance = 0;
                } else {
                    sample.segment_length = canyon::Distance(sample.position, lastSample.position);
                    sample.total_distance = lastSample.total_distance + sample.segment_length;
                }
                lastSample = sample;
            }
            cacheData->total_length = lastSample.total_distance;
        }
    }

    void Deinit(BehaviourTarget const& target) override {
        target.registry.erase<ComponentCurveCache>(target.entity);
    }

    void Update(BehaviourTarget const& target, uint32_t ticks) override {
        auto& registry = target.registry;
        auto entity = target.entity;
        auto& behaviour = target.behaviourComponent;
        auto speed = target.behaviourComponent.m_currentState->speed;
        auto& position = target.positionComponent;

        auto& cacheData = registry.get<ComponentCurveCache>(entity);
        auto const distanceTravelled = speed * (static_cast<float>(behaviour.m_stateTime) / 1000.0f);
        size_t index = 0;
        for (size_t i = 1; i < cacheData.samples.size(); ++i) {
            if (distanceTravelled < cacheData.samples[i].total_distance) {
                break;
            }
            index = i;
        }
        auto const& pastNode = cacheData.samples[index];

        position.m_lastPosition = position.m_position;
        position.m_position = pastNode.position;

        if (index < (cacheData.samples.size() - 1)) {
            auto const& nextNode = cacheData.samples[index + 1];
            float const remainder = distanceTravelled - pastNode.total_distance;
            if (remainder > 0) {
                float const factor = remainder / nextNode.segment_length;
                auto const delta = nextNode.position - pastNode.position;
                position.m_position += delta * factor;
            }
        }
    }
};

class BehaviourHandlerSpline : public BehaviourHandler {
public:
    void Init(BehaviourTarget const& target) override {
        auto& registry = target.registry;
        auto entity = target.entity;
        auto& behaviour = target.behaviourComponent;
        auto& position = target.positionComponent;

        auto* cacheData = registry.try_get<ComponentSplineCache>(entity);
        if (cacheData == nullptr) {
            cacheData = &registry.emplace<ComponentSplineCache>(entity);
            cacheData->m_offset = position.m_position;
            cacheData->m_lut = BuildArcLengthTable(behaviour.m_currentState->points);
        }
    }

    void Deinit(BehaviourTarget const& target) override {
        target.registry.erase<ComponentSplineCache>(target.entity);
    }

    void Update(BehaviourTarget const& target, uint32_t ticks) override {
        auto& registry = target.registry;
        auto entity = target.entity;
        auto& behaviour = target.behaviourComponent;
        auto& position = target.positionComponent;

        auto* cacheData = registry.try_get<ComponentSplineCache>(entity);

        float const speed = behaviour.m_currentState->speed;
        auto const distanceTravelled = speed * (static_cast<float>(behaviour.m_stateTime) / 1000.0f);
        auto const sample = SamplePathByDistance(behaviour.m_currentState->points, cacheData->m_lut, distanceTravelled);

        position.m_lastPosition = position.m_position;
        position.m_position = cacheData->m_offset + sample;
    }
};

std::map<BehaviourType, std::shared_ptr<BehaviourHandler>> const BehaviourHandlers{
    { BehaviourType::Hold, std::make_shared<BehaviourHandlerHold>() },
    { BehaviourType::Line, std::make_shared<BehaviourHandlerLine>() },
    { BehaviourType::Target, std::make_shared<BehaviourHandlerTarget>() },
    { BehaviourType::Wave, std::make_shared<BehaviourHandlerWave>() },
    { BehaviourType::Spline, std::make_shared<BehaviourHandlerSpline>() }
};

void SystemBehaviour::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto view = registry.view<ComponentBehaviour, ComponentPosition, ComponentVelocity>();

    for (auto const& [entity, behaviour, position, velocity] : view.each()) {
        if (behaviour.m_stateIdx < behaviour.m_data.states.size()) {
            auto pendingTicks = ticks;
            while (pendingTicks > 0) {
                if (behaviour.m_currentState != nullptr) {
                    auto const maxTime = behaviour.m_currentState->duration;
                    auto const remainingTime = maxTime - behaviour.m_stateTime;
                    auto const usedTicks = std::min(remainingTime, pendingTicks);
                    if (maxTime > 0) {
                        behaviour.m_stateTime += usedTicks;
                        pendingTicks -= usedTicks;
                    } else {
                        behaviour.m_stateTime += pendingTicks;
                        pendingTicks = 0;
                    }
                    auto const handlerIt = BehaviourHandlers.find(behaviour.m_currentState->type);
                    if (std::end(BehaviourHandlers) != handlerIt) {
                        handlerIt->second->Update({ registry, entity, behaviour, position, velocity },
                                                  usedTicks);
                    }
                    if (pendingTicks > 0) {
                        handlerIt->second->Deinit({ registry, entity, behaviour, position, velocity });
                        ++behaviour.m_stateIdx;
                    }
                }
                if (pendingTicks > 0) {
                    // transition
                    behaviour.m_currentState = nullptr;
                    behaviour.m_stateTime = 0;
                    if (behaviour.m_stateIdx < behaviour.m_data.states.size()) {
                        behaviour.m_currentState = &behaviour.m_data.states[behaviour.m_stateIdx];
                        auto const it = BehaviourHandlers.find(behaviour.m_currentState->type);
                        if (std::end(BehaviourHandlers) != it) {
                            it->second->Init({ registry, entity, behaviour, position, velocity });
                        }
                    }
                }
            }
        }
    }
}
