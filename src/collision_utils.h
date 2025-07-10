#pragma once

#include <moth_ui/utils/vector.h>

/**
 * Does a sweeping test between two radii from their last position to their current position.
 * Returns the factor between last position and new position where the collision occurred or 1.0f if no
 * collision
 */
float SweepTest(moth_ui::FloatVec2 const& obj1Pos, moth_ui::FloatVec2 const& obj1LastPos, float obj1Radius,
                moth_ui::FloatVec2 const& obj2Pos, moth_ui::FloatVec2 const& obj2LastPos, float obj2Radius);
