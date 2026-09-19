# Tasks

The task list for TinCan. The process, the status values and the field
definitions are in the shared workflow, `/home/mcotton/Development/moth/.claude/tasks/workflow.md`.

- `/task-new <description>` adds a task.
- `/task-planning [IDs]` reviews tasks.
- `/task-session` implements reviewed tasks.

Session reports are in [sessions/](sessions/). `/task-planning` moves finished tasks
to [tasks-done.md](tasks-done.md).

## Tasks

### [todo] T-008 Add a smoke launch check

**Review:** reviewed 2026-09-19

**Depends on:** none

**Goal:**
TinCan has no launch check, so a session can confirm only that the build
succeeds. Every later task in this list changes something visual, so a session
has no way to tell whether it broke the game. Port the check moth_sprite uses
(`../../moth_sprite/tools/smoke_launch.sh` and `tools/close_window.py`) so
sessions can verify the game starts, runs and shuts down cleanly.

This task comes first because it raises the verification floor for everything
after it.

**Requirements:**
- [ ] `tools/smoke_launch.sh` starts the Debug build, waits for the window, lets
      it run a few seconds, closes it with `WM_DELETE_WINDOW` so normal shutdown
      runs, and reports pass or fail.
- [ ] It runs the game in a temporary directory, because the app writes
      `imgui.ini` to the current directory. Unlike moth_sprite, TinCan also needs
      `assets/`, `data/` and `tiled_project/` beside it, so the script links or
      copies them in.
- [ ] It passes when the app exits with code 0 and the log has no `[warning]` or
      `[error]` lines. Debug enables the Vulkan validation layers, so this is the
      check that catches renderer regressions.
- [ ] Two known shutdown validation errors are allowed, named by VUID so they
      cannot hide anything else: `VUID-vkDestroyBuffer-buffer-00922` and
      `VUID-vkDestroySampler-sampler-01082`. Any other validation error still
      fails the check. The allowance is removed once moth_toolkit T-002 is
      released and TinCan picks up the new `moth_graphics`.
- [ ] It prints a clear pass or fail line, exits 0 on pass, and exits 2 when it
      cannot run at all (no display, no build, missing tool).
- [ ] The window title is set to something specific enough to match. It is
      currently `"Example"` (`src/main.cpp:6`).
- [ ] `docs/task-profile.md` is updated to name the launch check instead of
      `none`.

**Out of scope:**
Automated tests. Adding a test framework to a project that has none is separate
work.

**Open questions:**
- Q: Does the game currently start and shut down without logging warnings or
  errors? A: It launches cleanly, but shutdown logs two Vulkan validation errors.
  They are a destruction-order problem in gfx, diagnosed as moth_toolkit T-002:
  `Graphics` destroys its buffers and the default image's sampler before the
  shaders whose destructor frees the descriptor sets pointing at them. Cosmetic,
  logged after the last frame. The check allows those two VUIDs by name. Any
  other warning or error the session finds is recorded in `Notes`, not silenced,
  and the task blocks if the check cannot otherwise pass against unmodified code.

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-001 Guard the path samplers against short point lists

**Review:** reviewed 2026-09-19

**Depends on:** none

**Goal:**
Spawning an editor-built spline crashes the application. `SampleCatmullRomPath`
(`src/utils_paths.h:14`) indexes `points.back()` and `points[0]` with no size
check, and `BuildArcLengthTable` reaches it with whatever the editor holds. Below
four points the sampler silently returns `points[0]` forever, so the enemy sits
still instead of moving. Stop the crash before anything is built on top of it.

Step 1 of [behaviour-paths-and-editor.md](behaviour-paths-and-editor.md).

**Requirements:**
- [ ] Zero, one, two and three points are each defined cases in the sampler, with
      no crash and no silent freeze.
- [ ] Fewer than two points is a no-op: the entity holds position for the state's
      duration, and a warning naming the behaviour is logged once, not per frame.
- [ ] Catmull-Rom endpoint padding happens in the sampler, not in `from_json`
      (`src/gamedata_behaviour.h:60`), so authored and loaded data agree.
- [ ] The behaviour editor's Spawn button is disabled while the current state is
      incomplete.

**Out of scope:**
The waypoint model itself. This task keeps the existing `Line`/`Wave`/`Spline`
types and only makes them safe.

**Open questions:**
- Q: What should a path with fewer than two waypoints do? A: A no-op that logs
  once, rather than a load-time error. The editor builds states incrementally, so
  a half-placed path has to be survivable at runtime.

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-004 Pick enemies and behaviours from the database in the editor

**Review:** reviewed 2026-09-19

**Depends on:** none

**Goal:**
The behaviour editor asks for sprite, radius and health (`InputEnemyData`,
`src/system_behaviour_editor.cpp:8`) instead of naming an enemy that already
exists. Replace those fields with a combo over the enemy database keys, and add a
second combo over the behaviour database keys that loads a named behaviour as the
starting point. Both follow the sprite combo already in that function.

Editing a behaviour that exists, rather than always starting blank, is most of
what makes the editor useful. This task has no dependencies and can land before
the model change.

Step 4 of [behaviour-paths-and-editor.md](behaviour-paths-and-editor.md).

**Requirements:**
- [ ] A combo selects which enemy from `data/enemy_database.json` to spawn.
- [ ] A combo loads a named behaviour from `data/behaviour_database.json` into
      the editor as a starting point.
- [ ] The typed sprite, radius and health fields are gone.
- [ ] Spawning uses the selected enemy's data, not a hand-built `EnemyData`.

**Out of scope:**
A general gamedata editor that builds selectors from the data structures. That is
noted as out of scope in the plan and is separate, larger work.

Saving, which is T-006. Loading a behaviour to edit does not imply writing it
back.

**Open questions:**

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-002 Replace the movement types with a waypoint path state

**Review:** reviewed 2026-09-19

**Depends on:** T-001

**Goal:**
Collapse `Line`, `Wave` and `Spline` into a single path state: a list of
waypoints, an interpolation between them, an explicit coordinate space and a
speed. One arc-length sampler serves both interpolations, so speed means the same
thing whatever the shape and the editor cannot draw a curve the game does not
fly. Coordinate space stops being implied by the type.

Existing behaviour data is test data and may be discarded, so
`data/behaviour_database.json` is rewritten by hand rather than migrated. It is
three entries, and none of them uses `Wave` or `Spline`, so nothing in the game
loses a behaviour it was using.

Step 2 of [behaviour-paths-and-editor.md](behaviour-paths-and-editor.md).

**Requirements:**
- [ ] A movement state carries `waypoints`, `interpolation` (`Linear` or
      `CatmullRom`), `speed` and `duration`, where `duration: 0` means "until the
      path completes".
- [ ] `space` (`SpawnRelative` or `World`) is a property of the behaviour, not of
      each state.
- [ ] `BehaviourStateData` stays one struct holding the union of every type's
      fields, with `from_json` branching per type as it does today.
- [ ] One arc-length sampler serves both interpolations.
- [ ] `Line`, `Wave` and `Spline` and their handlers are gone
      (`src/system_behaviour.cpp:123`–`269`, `src/gamedata_behaviour.h:6`).
- [ ] `Target` and `Hold` are unchanged.
- [ ] `data/behaviour_database.json` is rewritten: `basic` becomes a two-waypoint
      linear path, `chase_player` and `in_and_hold` behave as they do today.

**Out of scope:**
Modifiers, which are T-003. Conditional transitions between states.

Removing the dead code this leaves behind (`SystemSplineEditor`,
`ComponentCurveCache`), which is T-009.

`data/spawner_database.json`. Its `"behaviour": "Spline"`, `"Straight"` and
`"Wave"` entries are behaviour *names*, not type names, and they already refer to
behaviours that do not exist. The whole spawner path is dead:
`SystemEnemySpawner::CreateSpawner` has no caller and `InitTestWorld()` is
commented out (`src/game_world.cpp:39`). Leave the file alone.

**Open questions:**
- Q: One struct or a variant per state type? A: Keep one struct. A variant is
  tidier but touches the loader, every handler and the editor at once, which
  would push this past one session.
- Q: Is `space` per state or per behaviour? A: Per behaviour. A path is authored
  as a shape relative to where the enemy enters, and one control is easier to
  present in the editor.
- Q: `SpawnMapEnemy` overrides `state.speed` for every state from the Tiled
  `speed` property (`src/system_map.cpp:99`). `Line` ignores speed, so today that
  property does nothing for `basic`; once `basic` is a linear path it will start
  applying. A: Accepted, this is the property doing what it says. The star map's
  pacing may shift and is checked by hand.

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-003 Add movement modifiers, starting with Wave

**Review:** reviewed 2026-09-19

**Depends on:** T-002

**Goal:**
A movement state carries a list of modifiers applied to the position the path
produced, so a wave over a two-waypoint linear path gives wavy movement and the
same wave over a curve gives a wavy curve. `Phase` lets formation members on one
shared behaviour stop moving in lockstep, and `Ease` shapes speed along a path
without bending its geometry.

Modifiers are a small fixed set of numbers. They do not read the world, do not
branch and cannot be nested — that limit is the point, not a shortcut. It is what
keeps this a way of describing shapes rather than a scripting language with a
worse interface than text.

Step 3 of [behaviour-paths-and-editor.md](behaviour-paths-and-editor.md).

**Requirements:**
- [ ] `Wave` applies oscillation perpendicular to the direction of travel, with
      amplitude and frequency.
- [ ] `Phase` offsets a modifier's cycle, so two entities on the same behaviour
      can be visibly out of step.
- [ ] `Ease` reshapes speed along the path without changing its geometry.
- [ ] Modifiers work over both `Linear` and `CatmullRom` paths.
- [ ] Modifiers apply in an order fixed by the engine, not by the order they are
      listed in the data.

**Out of scope:**
Modifiers that read world state or take expressions. Nested modifiers.
Reorderable modifier lists.

**Open questions:**
- Q: Is modifier order authored or fixed? A: Fixed by the engine. One less thing
  to get wrong in the editor, and most stacks are order-independent anyway.
- Q: Should `Wave` reproduce the old `Wave` type exactly? A: No. Nothing in
  `data/` used the old type, and T-002 removes it, so there is nothing left to
  compare against. The requirement is that it looks right, checked by hand.

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-009 Remove the dead spline and curve code

**Review:** reviewed 2026-09-19

**Depends on:** T-003

**Goal:**
T-002 and T-003 leave code behind that nothing reaches. `SystemSplineEditor`'s
only call site is already commented out (`src/game_world.cpp:80`),
`ComponentCurveCache` (`src/system_behaviour.h:15`) is used only by the `Wave`
handler that T-002 deletes, and `src/system_behaviour.cpp` carries a commented-out
`BehaviourSpline` from an earlier design (lines 35–99).

Kept separate from T-002 and T-003 so that cleanup is not smuggled into a feature
change, per the workspace `CLAUDE.md`.

**Requirements:**
- [ ] Code that nothing reaches after T-003 is removed, along with its entries in
      `SOURCES` in `CMakeLists.txt`.
- [ ] `DrawSpline` / `src/system_debug_spline.*` is kept if the editor's path
      preview still uses it, and removed if not. Check, do not assume.
- [ ] Commented-out code left from the old design is removed.
- [ ] No behaviour changes.

**Out of scope:**
`data/spawner_database.json` and the unused spawner path
(`SystemEnemySpawner::CreateSpawner`, `InitTestWorld`). That is dead too, but it
is a separate question about whether spawners come back, not part of this
cleanup.

**Open questions:**

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-005 Edit waypoints with the mouse in the game view

**Review:** reviewed 2026-09-19

**Depends on:** T-002, T-004

**Goal:**
Waypoints are typed in as float pairs, one `InputFloat2` per point
(`src/system_behaviour_editor.cpp:28`), which is the main reason the editor is
hard to work with. Draw waypoints as handles into the world surface next to the
path preview, and edit them directly: click empty space to append, drag a handle
to move, right-click a handle to delete, with the path redrawing live.

This needs a window-to-world transform. `GameLayer::Draw`
(`src/game_layer.cpp:59`–`67`) already computes the scaling and letterbox offset
every frame and then discards it; lifting it into something both the draw and the
editor can ask for is the enabling change.

Editing in the game view rather than a separate canvas is deliberate: the
judgement being made is whether a movement feels right at the speed and scale the
player sees.

Step 5 of [behaviour-paths-and-editor.md](behaviour-paths-and-editor.md).

**Requirements:**
- [ ] The window-to-world transform is available to both the draw and the editor
      rather than recomputed or discarded.
- [ ] Waypoints draw as handles in the game view.
- [ ] Click on empty space appends a waypoint, drag moves one, right-click
      deletes one.
- [ ] Mouse input for editing is ignored when ImGui wants the mouse, so clicking
      in the editor panel does not place a waypoint.
- [ ] The path preview redraws live while editing.
- [ ] The numeric fields remain, for precise nudging and to show what is
      selected.

**Out of scope:**
Saving, which is T-006. Editing anything other than waypoints with the mouse.

**Open questions:**

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-006 Save edited behaviours back to the database

**Review:** reviewed 2026-09-19

**Depends on:** T-002, T-005

**Goal:**
The editor edits function-local `static` data in `Draw`
(`src/system_behaviour_editor.cpp:108`), so nothing survives the session. That
caps how much T-004 and T-005 are worth. The gamedata structs are load-only, so
the movement state needs a serialiser to match its loader, and the editor writes
the edited behaviour back to `data/behaviour_database.json` under a name.

Step 6 of [behaviour-paths-and-editor.md](behaviour-paths-and-editor.md).

**Requirements:**
- [ ] The movement state and its modifiers have serialisers matching their
      loaders.
- [ ] Saving loads the whole of `data/behaviour_database.json`, replaces the
      named behaviour, and writes the file back.
- [ ] Saving under a new name adds a behaviour rather than replacing one.
- [ ] A behaviour saved from the editor reloads on the next run and moves
      identically.

**Out of scope:**
Saving enemy, sprite, weapon or any other database. Preserving the file's
formatting, ordering or comments — a full rewrite is accepted.

**Open questions:**
- Q: Whole file or only the named behaviour? A: Rewrite the whole file. Simple
  and predictable; the cost is that hand-written formatting and ordering are
  lost, which is accepted.

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-007 Preview a path with a scrubbed ghost

**Review:** reviewed 2026-09-19

**Depends on:** T-002, T-005

**Goal:**
A shape should be readable without spawning an enemy and watching it. Scrub a
ghost along the path, with direction and time markers, so the loop stops being
"spawn, watch, guess". Playback at the real speed is what tells you whether a
movement feels right; scrubbing is what lets you inspect a moment.

Ordered after T-005 so it is built against the finished editor rather than
reworked.

Step 7 of [behaviour-paths-and-editor.md](behaviour-paths-and-editor.md).

**Requirements:**
- [ ] A ghost can be scrubbed along the current path with a slider.
- [ ] The ghost can play back at the state's real speed, with play and pause.
- [ ] The preview shows direction along the path.
- [ ] The preview shows time markers.
- [ ] The ghost reflects modifiers, so a wave shows as a wave.

**Out of scope:**
Previewing a whole behaviour across several states. This is one state's path.

**Open questions:**
- Q: Scrub only, or playback too? A: Both. Playback is what makes the preview
  worth having; scrubbing is for inspecting a moment.

**Notes:**

**Commits:**

**Manual verification:**

## Discovered
