#pragma once

#include <moth_graphics/utils/rect.h>
#include <moth_graphics/utils/vector.h>
#include <fmt/core.h>

template<typename T, int Dim>
struct fmt::formatter<moth_graphics::Vector<T, Dim>> : fmt::formatter<std::string>
{
    auto format(moth_graphics::Vector<T, Dim> vec, format_context &ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "({}, {})", vec.x, vec.y);
    }
};

template<typename T>
struct fmt::formatter<moth_graphics::Rect<T>> : fmt::formatter<std::string>
{
    auto format(moth_graphics::Rect<T> rect, format_context &ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "({}, {})", rect.topLeft, rect.bottomRight);
    }
};

