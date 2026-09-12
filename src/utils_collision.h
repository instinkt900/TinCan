#pragma once

#include <cstdlib>
#include <moth/graphics/utils/vector.h>
#include <moth/graphics/utils/rect.h>

/**
 * Does a sweeping test between two radii from their last position to their current position.
 * Returns the factor between last position and new position where the collision occurred or 1.0f if no
 * collision
 */
float SweepTest(moth::gfx::FloatVec2 const& obj1Pos, moth::gfx::FloatVec2 const& obj1LastPos, float obj1Radius,
                moth::gfx::FloatVec2 const& obj2Pos, moth::gfx::FloatVec2 const& obj2LastPos, float obj2Radius);

bool IntersectSegmentWithLine(moth::gfx::FloatVec2 const& p1, moth::gfx::FloatVec2 const& p2,
                              moth::gfx::FloatVec2 const& a, moth::gfx::FloatVec2 const& b,
                              moth::gfx::FloatVec2& out);

bool GetExitPoint(moth::gfx::FloatVec2 p1, moth::gfx::FloatVec2 p2, const moth::gfx::FloatRect& bounds,
                  moth::gfx::FloatVec2& out);
