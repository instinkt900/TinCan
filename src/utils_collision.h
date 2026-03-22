#pragma once

#include <cstdlib>
#include <moth_graphics/utils/vector.h>
#include <moth_graphics/utils/rect.h>

/**
 * Does a sweeping test between two radii from their last position to their current position.
 * Returns the factor between last position and new position where the collision occurred or 1.0f if no
 * collision
 */
float SweepTest(moth_graphics::FloatVec2 const& obj1Pos, moth_graphics::FloatVec2 const& obj1LastPos, float obj1Radius,
                moth_graphics::FloatVec2 const& obj2Pos, moth_graphics::FloatVec2 const& obj2LastPos, float obj2Radius);

bool IntersectSegmentWithLine(moth_graphics::FloatVec2 const& p1, moth_graphics::FloatVec2 const& p2,
                              moth_graphics::FloatVec2 const& a, moth_graphics::FloatVec2 const& b,
                              moth_graphics::FloatVec2& out);

bool GetExitPoint(moth_graphics::FloatVec2 p1, moth_graphics::FloatVec2 p2, const moth_graphics::FloatRect& bounds,
                  moth_graphics::FloatVec2& out);
