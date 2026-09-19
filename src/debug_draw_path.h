#pragma once

#include "utils_paths.h"
#include <moth/graphics/graphics/igraphics.h>

// draws the path the game would fly, so the preview and the movement use the same sampler
void DrawPath(moth::gfx::IGraphics& graphics, std::vector<moth::gfx::FloatVec2> const& points,
              PathInterpolation interpolation, moth::gfx::FloatVec2 const& offset = { 0, 0 });

// draws a handle at each waypoint, with the selected one picked out
void DrawWaypointHandles(moth::gfx::IGraphics& graphics, std::vector<moth::gfx::FloatVec2> const& points,
                         moth::gfx::FloatVec2 const& offset, int selectedIndex);

// how close, in world units, the mouse has to be to a waypoint to take hold of it
float constexpr WaypointHandleRadius = 12.0f;
