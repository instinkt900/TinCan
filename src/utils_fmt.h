#pragma once

#include <moth/graphics/utils/rect.h>
#include <moth/graphics/utils/vector.h>
#include <fmt/core.h>

template<typename T, int Dim>
struct fmt::formatter<moth::gfx::Vector<T, Dim>> : fmt::formatter<std::string>
{
    auto format(moth::gfx::Vector<T, Dim> vec, format_context &ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "({}, {})", vec.x, vec.y);
    }
};

template<typename T>
struct fmt::formatter<moth::gfx::Rect<T>> : fmt::formatter<std::string>
{
    auto format(moth::gfx::Rect<T> rect, format_context &ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "({}, {})", rect.topLeft, rect.bottomRight);
    }
};

