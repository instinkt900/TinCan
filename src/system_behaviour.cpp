#include "system_behaviour.h"
#include "system_lifetime.h"
#include "system_movement.h"
#include <cassert>
#include <entt/entt.hpp>
#include <moth_ui/utils/vector_utils.h>
#include <spdlog/spdlog.h>

template <typename T>
T GetParameter(BehaviourParameterList const& parameterList, std::string const& name, T const defaultValue) {
    auto entry = parameterList.find(name);
    if (entry == parameterList.end()) {
        return defaultValue;
    }
    return std::get<T>(entry->second);
}

void BehaviourStraight(entt::registry& registry, entt::entity entity, ComponentBehaviour& behaviour,
                       ComponentPosition& position, ComponentLifetime& lifetime) {
    float const speed = GetParameter(behaviour.m_parameters, "speed", 100.0f);
    float const t = static_cast<float>(lifetime.m_msAlive) / 1000.0f;
    auto const newPosition = behaviour.m_offset + moth_ui::FloatVec2{ 0, t * speed };
    position.m_lastPosition = position.m_position;
    position.m_position = newPosition;
}

void BehaviourWave(entt::registry& registry, entt::entity entity, ComponentBehaviour& behaviour,
                   ComponentPosition& position, ComponentLifetime& lifetime) {
    float const freq = GetParameter(behaviour.m_parameters, "frequency", 1.0f);
    float const amp = GetParameter(behaviour.m_parameters, "amplitude", 1.0f);
    float const speed = GetParameter(behaviour.m_parameters, "speed", 100.0f);

    auto* cacheData = registry.try_get<ComponentCurveCache>(entity);
    if (cacheData == nullptr) {
        cacheData = &registry.emplace<ComponentCurveCache>(entity);

        float const MaxT = 1500.0f;
        float const TStep = 50.0f;

        CurveSample lastSample;
        for (float t = 0; t < MaxT; t += TStep) {
            float const tf = t / 1000.0f;
            auto& sample = cacheData->samples.emplace_back();
            sample.position.x = std::sin(tf * freq) * amp;
            sample.position.y = t;
            if (t == 0) {
                sample.segment_length = 0;
                sample.total_distance = 0;
            } else {
                sample.segment_length = moth_ui::Distance(sample.position, lastSample.position);
                sample.total_distance = lastSample.total_distance + sample.segment_length;
            }
            lastSample = sample;
        }
        cacheData->total_length = lastSample.total_distance;
    }

    auto const distanceTravelled = speed * (static_cast<float>(lifetime.m_msAlive) / 1000.0f);
    size_t index = 0;
    for (size_t i = 1; i < cacheData->samples.size(); ++i) {
        if (distanceTravelled < cacheData->samples[i].total_distance) {
            break;
        }
        index = i;
    }
    auto const& pastNode = cacheData->samples[index];

    position.m_lastPosition = position.m_position;
    position.m_position = behaviour.m_offset + pastNode.position;

    if (index < (cacheData->samples.size() - 1)) {
        auto const& nextNode = cacheData->samples[index + 1];
        float const remainder = distanceTravelled - pastNode.total_distance;
        if (remainder > 0) {
            float const factor = remainder / nextNode.segment_length;
            auto const delta = nextNode.position - pastNode.position;
            position.m_position += delta * factor;
        }
    }
}

using BehaviourFunc = void (*)(entt::registry&, entt::entity, ComponentBehaviour&, ComponentPosition&,
                               ComponentLifetime&);

std::map<std::string, BehaviourFunc> const Funcs{ { "straight", BehaviourStraight },
                                                  { "wave", BehaviourWave } };


void SystemBehaviour::Update(entt::registry& registry, uint32_t ticks) {
    auto view = registry.view<ComponentBehaviour, ComponentPosition, ComponentLifetime>();

    for (auto [entity, behaviour, position, lifetime] : view.each()) {
        auto entry = Funcs.find(behaviour.m_behaviourName);
        if (entry == Funcs.end()) {
            spdlog::error("Unknown behaviour name {}", behaviour.m_behaviourName);
            continue;
        }
        entry->second(registry, entity, behaviour, position, lifetime);
    }
}
