#pragma once

#include "system_behaviour.h"
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

// Draws the line the state actually flies, modifiers and all. Used at full strength for the state being
// edited, and faintly for the others, so a path can be placed against the rest of the behaviour.
void DrawFlownPath(moth::gfx::IGraphics& graphics, BehaviourStateData const& state, ArcLengthLUT const& lut,
                   moth::gfx::FloatVec2 const& offset, moth::gfx::Color const& color);

// The colour the states that are not being edited are drawn in. Opaque rather than faded through alpha,
// because the editor draws while the blend mode is Replace, which would ignore the alpha and give a fully
// bright line instead of a faint one.
inline moth::gfx::Color ReferencePathColor() { return moth::gfx::FromARGB(0xFF707070); }

// Draws what the state actually flies: the path with its modifiers applied, arrows along the direction of
// travel, a tick for each second at the state's speed, and a ghost at the scrubbed point. progress is the
// fraction of the path travelled, before any easing.
void DrawPathStatePreview(moth::gfx::IGraphics& graphics, BehaviourStateData const& state,
                          ArcLengthLUT const& lut, moth::gfx::FloatVec2 const& offset, float progress);
