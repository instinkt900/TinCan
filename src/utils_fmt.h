#pragma once

#include <canyon/utils/rect.h>
#include <canyon/utils/vector.h>
#include <fmt/core.h>

template<typename T, int Dim>
struct fmt::formatter<canyon::Vector<T, Dim>> : fmt::formatter<std::string>
{
    auto format(canyon::Vector<T, Dim> vec, format_context &ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "({}, {})", vec.x, vec.y);
    }
};

template<typename T>
struct fmt::formatter<canyon::Rect<T>> : fmt::formatter<std::string>
{
    auto format(canyon::Rect<T> rect, format_context &ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "({}, {})", rect.topLeft, rect.bottomRight);
    }
};

