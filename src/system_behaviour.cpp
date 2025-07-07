#include "system_behaviour.h"
#include "system_lifetime.h"
#include "system_movement.h"
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

        float const MaxT = 5000.0f;
        float const TStep = 20.0f;

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

    float const totalDistance = speed * (static_cast<float>(lifetime.m_msAlive) / 1000.0f);
    auto const it = std::lower_bound(cacheData->samples.begin(), cacheData->samples.end(), totalDistance,
                               [](auto const& it, auto const& value) { return it.total_distance <= value; });
    size_t const index = std::distance(cacheData->samples.begin(), it);

    position.m_lastPosition = position.m_position;
    position.m_position = behaviour.m_offset + cacheData->samples[index].position;

    float const remainder = cacheData->samples[index].total_distance - totalDistance;
    if (remainder > 0) {
        float const factor = remainder / (cacheData->samples[index].segment_length);
        auto const delta = cacheData->samples[index].position - cacheData->samples[index+1].position;
        position.m_position += delta * factor;
    }
}

SystemBehaviour::SystemBehaviour() {
    m_behaviours["straight"] = BehaviourStraight;
    m_behaviours["wave"] = BehaviourWave;
}

void SystemBehaviour::Update(entt::registry& registry, uint32_t ticks) {
    auto view = registry.view<ComponentBehaviour, ComponentPosition, ComponentLifetime>();

    for (auto [entity, behaviour, position, lifetime] : view.each()) {
        auto entry = m_behaviours.find(behaviour.m_behaviourName);
        if (entry == m_behaviours.end()) {
            spdlog::error("Unknown behaviour name {}", behaviour.m_behaviourName);
            continue;
        }
        entry->second(registry, entity, behaviour, position, lifetime);
    }
}
