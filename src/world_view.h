#pragma once

#include <moth/graphics/utils/vector.h>

// Where the world image sits inside the window, and how much it was scaled to fit. The draw works this out
// once a frame to place the image; the behaviour editor asks for it to turn a mouse position into a point
// in the world.
struct WorldViewTransform {
    moth::gfx::FloatVec2 m_offset{ 0, 0 };
    float m_scale{ 1.0f };

    moth::gfx::FloatVec2 WindowToWorld(moth::gfx::FloatVec2 const& windowPosition) const {
        return (windowPosition - m_offset) * (1.0f / m_scale);
    }

    moth::gfx::FloatVec2 WorldToWindow(moth::gfx::FloatVec2 const& worldPosition) const {
        return (worldPosition * m_scale) + m_offset;
    }
};
