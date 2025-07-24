#pragma once

#include <random>

template <typename T>
inline T Random(T min, T max) {
    static thread_local std::mt19937 rng(std::random_device{}());

    if constexpr (std::is_integral_v<T>) {
        std::uniform_int_distribution<T> dist(min, max);
        return dist(rng);
    } else if constexpr (std::is_floating_point_v<T>) {
        std::uniform_real_distribution<T> dist(min, max);
        return dist(rng);
    } else {
        static_assert(std::is_arithmetic_v<T>, "Random<T> requires arithmetic type");
    }
}
