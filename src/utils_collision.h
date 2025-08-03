#pragma once

#include <cstdlib>
#include <canyon/utils/vector.h>
#include <canyon/utils/rect.h>

/**
 * Does a sweeping test between two radii from their last position to their current position.
 * Returns the factor between last position and new position where the collision occurred or 1.0f if no
 * collision
 */
float SweepTest(canyon::FloatVec2 const& obj1Pos, canyon::FloatVec2 const& obj1LastPos, float obj1Radius,
                canyon::FloatVec2 const& obj2Pos, canyon::FloatVec2 const& obj2LastPos, float obj2Radius);

bool IntersectSegmentWithLine(canyon::FloatVec2 const& p1, canyon::FloatVec2 const& p2,
                              canyon::FloatVec2 const& a, canyon::FloatVec2 const& b,
                              canyon::FloatVec2& out);

bool GetExitPoint(canyon::FloatVec2 p1, canyon::FloatVec2 p2, const canyon::FloatRect& bounds,
                  canyon::FloatVec2& out);
