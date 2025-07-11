#pragma once

#include <map>

enum class PassiveType {
    WeaponCooldown,
    WeaponCooldownBurst,
    WeaponDamage,
    WeaponBurst,
};

struct ComponentPassives {
    std::map<PassiveType, float> m_value;
};

static std::map<PassiveType, float> const DefaultPassiveValue{ { PassiveType::WeaponCooldown, 1.0f },
                                                               { PassiveType::WeaponCooldownBurst, 1.0f },
                                                               { PassiveType::WeaponDamage, 1.0f },
                                                               { PassiveType::WeaponBurst, 0.0f } };

inline float PassiveValue(ComponentPassives const& passives, PassiveType type) {
    auto const it = passives.m_value.find(type);
    if (it == passives.m_value.end()) {
        auto const defaultIt = DefaultPassiveValue.find(type);
        if (defaultIt != DefaultPassiveValue.end()) {
            return defaultIt->second;
        }
        return 0;
    }
    return it->second;
}

inline float PassiveAccumulate(PassiveType type, float current, float value) {
    switch (type) {
        case PassiveType::WeaponCooldown:
            return current * (1.0f - value);
        case PassiveType::WeaponBurst:
            return current + value;
        default:
            return current * value;
    }
}

