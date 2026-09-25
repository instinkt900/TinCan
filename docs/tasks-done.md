# Finished tasks

Tasks for TinCan that are finished and whose commits are in the branch. The
process, the status values and the field definitions are in the shared workflow,
`/home/mcotton/Development/moth/.claude/tasks/workflow.md`. The live list is
[tasks.md](tasks.md).

### [done] T-008 Add a smoke launch check

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
- [x] `tools/smoke_launch.sh` starts the Debug build, waits for the window, lets
      it run a few seconds, closes it with `WM_DELETE_WINDOW` so normal shutdown
      runs, and reports pass or fail.
- [x] It runs the game in a temporary directory, because the app writes
      `imgui.ini` to the current directory. Unlike moth_sprite, TinCan also needs
      `assets/`, `data/` and `tiled_project/` beside it, so the script links or
      copies them in.
- [x] It passes when the app exits with code 0 and the log has no `[warning]` or
      `[error]` lines. Debug enables the Vulkan validation layers, so this is the
      check that catches renderer regressions.
- [x] Two known shutdown validation errors are allowed, named by VUID so they
      cannot hide anything else: `VUID-vkDestroyBuffer-buffer-00922` and
      `VUID-vkDestroySampler-sampler-01082`. Any other validation error still
      fails the check. The allowance is removed once moth_toolkit T-002 is
      released and TinCan picks up the new `moth_graphics`.
- [x] It prints a clear pass or fail line, exits 0 on pass, and exits 2 when it
      cannot run at all (no display, no build, missing tool).
- [x] The window title is set to something specific enough to match. It is
      currently `"Example"` (`src/main.cpp:6`).
- [x] `docs/task-profile.md` is updated to name the launch check instead of
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
The log records that the validation layers produce span several lines and the
VUID is on a continuation line, not on the line carrying `[error]`, so the check
reads the log as records rather than lines and allows a record that names one of
the two VUIDs. The two allowed shutdown errors are the only ones the check saw.

One other error was seen, once, in an early run where the close request went
astray and the game was left playing for twenty seconds:
`[error] Unable to locate player` from `SystemPickup::Update`
(`src/system_pickup.cpp:143`). It is logged when the player entity is gone, and
the loop that follows it then dereferences a null `playerPosition` if any pickup
is alive. Not silenced and not fixed here, because it is outside this task.
Recorded under `## Discovered`.

That early run also showed that a close request sent while the window manager is
still taking the window over can be dropped, so the check sends it a second time
after three seconds before it gives up. The check then passed three runs in a
row.

**Commits:**
- f12c1a7 test(T-008): add a smoke launch check

**Manual verification:**
- Run `tools/smoke_launch.sh` and confirm it prints `SMOKE LAUNCH PASSED`.
- Run the game normally and confirm the window is titled `TinCan`.

### [done] T-001 Guard the path samplers against short point lists

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
- [x] Zero, one, two and three points are each defined cases in the sampler, with
      no crash and no silent freeze.
- [x] Fewer than two points is a no-op: the entity holds position for the state's
      duration, and a warning naming the behaviour is logged once, not per frame.
- [x] Catmull-Rom endpoint padding happens in the sampler, not in `from_json`
      (`src/gamedata_behaviour.h:60`), so authored and loaded data agree.
- [x] The behaviour editor's Spawn button is disabled while the current state is
      incomplete.

**Out of scope:**
The waypoint model itself. This task keeps the existing `Line`/`Wave`/`Spline`
types and only makes them safe.

**Open questions:**
- Q: What should a path with fewer than two waypoints do? A: A no-op that logs
  once, rather than a load-time error. The editor builds states incrementally, so
  a half-placed path has to be survivable at runtime.

**Notes:**
The sampler now defines every length: empty is the origin, one point is that
point, two points are the straight line between them, and three or more are a
Catmull-Rom spline that clamps its control indices to the ends of the list. That
clamping is the endpoint padding, so the two duplicated points `from_json` used
to insert are gone and a loaded path holds the points that were authored.
`GetTForDistance` also returns early on a path that covers no distance, which
used to divide by zero and put an entity at a NaN position.

Assumption: "the current state is incomplete" is read as the behaviour that
Spawn would spawn, so the button is disabled when the behaviour has no states or
any of its states is incomplete, with a line beside it saying why. The "New
state" being typed in below is not part of the behaviour yet and does not count.
Incomplete means `Undefined` type, a `Target` with no target type, or a `Spline`
with fewer than two points.

Assumption: the warning names the behaviour by its state index and label, because
`BehaviourData` carries no name of its own; the name a behaviour is loaded under
is not kept in `ComponentBehaviour`.

**Commits:**
- 361e3bc fix(T-001): guard the path samplers against short point lists

**Manual verification:**
- Open the behaviour editor (F2). With no states added, Spawn is disabled.
- Add a Spline state with one point. Spawn stays disabled.
- Add a second point. Spawn enables, and the spawned enemy flies the path drawn
  in the preview.
- Set a spawned spline behaviour's state to a single point in the editor and
  confirm the enemy holds position rather than freezing the game, with one
  warning in the log rather than one per frame.

### [done] T-004 Pick enemies and behaviours from the database in the editor

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
- [x] A combo selects which enemy from `data/enemy_database.json` to spawn.
- [x] A combo loads a named behaviour from `data/behaviour_database.json` into
      the editor as a starting point.
- [x] The typed sprite, radius and health fields are gone.
- [x] Spawning uses the selected enemy's data, not a hand-built `EnemyData`.

**Out of scope:**
A general gamedata editor that builds selectors from the data structures. That is
noted as out of scope in the plan and is separate, larger work.

Saving, which is T-006. Loading a behaviour to edit does not imply writing it
back.

**Open questions:**

**Notes:**
The two combos share one templated helper over `GameData::Get<T>()->Keys()`,
which is how the sprite combo it replaces already worked. The helper reports the
frame the selection changes, so the behaviour is copied into the editor once on
selection rather than every frame: loading a behaviour is a starting point for
editing, not a live link to the database.

Assumption: the Spawn button, which T-001 disables for an unfinished behaviour,
is also disabled until an enemy is chosen, with a line beside it saying which of
the two is missing. Spawning with no enemy selected would put an entity in the
world with no sprite, no body and no health.

**Commits:**
- f30d0d8 feat(T-004): pick enemies and behaviours from the database in the editor

**Manual verification:**
- Open the behaviour editor (F2). The Enemy combo lists the entries of
  `data/enemy_database.json` and the Behaviour combo lists those of
  `data/behaviour_database.json`.
- With nothing chosen, Spawn is disabled and says an enemy is needed.
- Choose `chase_player` in the Behaviour combo and confirm its states appear in
  the editor below.
- Choose an enemy and spawn: the enemy that appears is the one named, with its
  own sprite, radius and health, and it follows the edited behaviour.

### [done] T-002 Replace the movement types with a waypoint path state

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
- [x] A movement state carries `waypoints`, `interpolation` (`Linear` or
      `CatmullRom`), `speed` and `duration`, where `duration: 0` means "until the
      path completes".
- [x] `space` (`SpawnRelative` or `World`) is a property of the behaviour, not of
      each state.
- [x] `BehaviourStateData` stays one struct holding the union of every type's
      fields, with `from_json` branching per type as it does today.
- [x] One arc-length sampler serves both interpolations.
- [x] `Line`, `Wave` and `Spline` and their handlers are gone
      (`src/system_behaviour.cpp:123`–`269`, `src/gamedata_behaviour.h:6`).
- [x] `Target` and `Hold` are unchanged.
- [x] `data/behaviour_database.json` is rewritten: `basic` becomes a two-waypoint
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
`PathInterpolation` lives in `utils_paths.h` beside the sampler that gives it
meaning, and `gamedata_behaviour.h` only adds its serialiser. `BuildArcLengthTable`
and `SamplePathByDistance` take the interpolation, so the same table serves both.

A state with no duration needed a way to end, so `BehaviourHandler` gained
`IsFinished`, which the state machine asks only of a state with no duration. The
path handler answers it by comparing the distance travelled with the length of
its arc-length table. The loop around it was rewritten to end a state and enter
the next one in the same pass rather than only when ticks were left over; a state
whose duration runs out now hands over in the same frame instead of the next.

`ComponentSplineCache` is renamed `ComponentPathCache`, and the editor preview's
`DrawSpline` becomes `DrawPath` and takes the interpolation, since Spline is no
longer a thing the game has.

Assumption: `basic` becomes `[0,0]` to `[0,1200]` in spawn-relative space at
speed 100. The old `Line` velocity was `[0,100]`, the world is 1080 high, so the
motion is the same and the enemy still leaves the bottom of the world before the
path completes.

Observed: a path drives position directly and leaves velocity at zero, which is
what the old `Spline` handler did too. `SystemMovement` runs after
`SystemBehaviour` and overwrites `m_lastPosition`, so a path-driven entity looks
stationary to the sweep tests for one frame. It was already so for splines, and
it costs only sub-frame accuracy because the projectile's own sweep still covers
the gap, so it is left alone rather than fixed inside this task.

**Commits:**
- eecc89c feat(T-002): replace the movement types with a waypoint path state

**Manual verification:**
- Play the star map. Enemies using `basic` fly down the screen as they did
  before, at the pace the map's `speed` property asks for.
- `chase_player` and `in_and_hold` behave as they did.
- In the editor (F2), a Path state offers waypoints, an interpolation and a
  speed, and the behaviour offers a space. Spawn one of each interpolation and
  confirm the enemy flies the line drawn in the preview.
- Spawn a path with `duration` 0 and confirm the enemy stops at the end of the
  path rather than continuing.

### [done] T-003 Add movement modifiers, starting with Wave

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
- [x] `Wave` applies oscillation perpendicular to the direction of travel, with
      amplitude and frequency.
- [x] `Phase` offsets a modifier's cycle, so two entities on the same behaviour
      can be visibly out of step.
- [x] `Ease` reshapes speed along the path without changing its geometry.
- [x] Modifiers work over both `Linear` and `CatmullRom` paths.
- [x] Modifiers apply in an order fixed by the engine, not by the order they are
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
The order the engine fixes is Phase, then Ease, then Wave: Phase only produces a
number the wave uses, Ease reshapes how far along the path the entity is, and
Wave displaces it across the direction of travel at that point. Several
modifiers of one kind accumulate in the order they are listed, which is the only
sense in which the listed order matters.

`Wave` measures frequency in cycles over the whole path rather than per second,
so the shape does not change with the speed, and it takes its direction from the
tangent of the path at that point. That is what makes it work the same over a
straight line and over a curve.

Assumption: `Phase` offsets by its `cycles` multiplied by the entity's place in
the formation it spawned with, which is the only way two entities on one shared
behaviour can differ. `ComponentBehaviour` gained `m_phaseIndex` and
`SpawnMapEnemy` sets it as it walks the formation offsets. A lone spawn, and
anything spawned from the editor, is index 0 and so is unaffected by Phase.

Assumption: a modifier whose type is still `Undefined` makes its state
incomplete, so the editor will not spawn a behaviour with a half-added modifier.

The editor's path preview draws the geometry of the path, not the modified
flight. T-007 owns showing what the entity actually flies, through its ghost.

**Commits:**
- 3fc1cce feat(T-003): add movement modifiers, starting with Wave

**Manual verification:**
- In the editor (F2), add a Path state with two waypoints and a Wave modifier
  with an amplitude and a frequency. The spawned enemy weaves across the line.
- Change the interpolation to CatmullRom with three or more waypoints. The same
  wave now weaves across the curve.
- Add an Ease modifier and confirm the enemy speeds up or slows down along the
  path without the path changing shape.
- In the star map, give a formation behaviour a Wave and a Phase modifier and
  confirm the members of the formation weave out of step with one another.

### [done] T-009 Remove the dead spline and curve code

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
- [x] Code that nothing reaches after T-003 is removed, along with its entries in
      `SOURCES` in `CMakeLists.txt`.
- [x] `DrawSpline` / `src/system_debug_spline.*` is kept if the editor's path
      preview still uses it, and removed if not. Check, do not assume.
- [x] Commented-out code left from the old design is removed.
- [x] No behaviour changes.

**Out of scope:**
`data/spawner_database.json` and the unused spawner path
(`SystemEnemySpawner::CreateSpawner`, `InitTestWorld`). That is dead too, but it
is a separate question about whether spawners come back, not part of this
cleanup.

**Open questions:**

**Notes:**
Checked rather than assumed: the editor's path preview does still use the draw
helper, so the file is kept. Everything else in it reached nothing —
`ComponentDebugSpline`, `imgui_ext::Details` for it and `SystemDebugSpline::Draw`
had no callers — so what is left is the one function, and the file is renamed
`src/debug_draw_path.*` to say so.

Removed: `src/system_spline_editor.*` and its `SOURCES` entry, its include and
its commented-out call in `game_world.cpp`; `CurveSample` and
`ComponentCurveCache` from `system_behaviour.h`; the commented-out
`BehaviourStraight`, `BehaviourWave` and `BehaviourSpline` from
`system_behaviour.cpp`; and the commented-out `DrawDebugCurves` and
`DrawDebugSplines` from `system_drawable.cpp`.

Left alone: the commented-out alternative `UpdateTargeting` in
`system_movement.cpp`, which is not part of this design, and the unused spawner
path, which the task puts out of scope.

**Commits:**
- 2dccd66 refactor(T-009): remove the dead spline and curve code

**Manual verification:**
- Nothing to check by hand beyond confirming the game still plays as it did:
  no behaviour was meant to change.

### [done] T-005 Edit waypoints with the mouse in the game view

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
- [x] The window-to-world transform is available to both the draw and the editor
      rather than recomputed or discarded.
- [x] Waypoints draw as handles in the game view.
- [x] Click on empty space appends a waypoint, drag moves one, right-click
      deletes one.
- [x] Mouse input for editing is ignored when ImGui wants the mouse, so clicking
      in the editor panel does not place a waypoint.
- [x] The path preview redraws live while editing.
- [x] The numeric fields remain, for precise nudging and to show what is
      selected.

**Out of scope:**
Saving, which is T-006. Editing anything other than waypoints with the mouse.

**Open questions:**

**Notes:**
The transform is a small `WorldViewTransform` in `src/world_view.h`, held by
`GameWorld`. `GameLayer::Draw` works it out at the top of the frame, before the
world draws, so the editor sees the current frame's transform rather than the
last one, and the draw then uses it to place the world image as it did before.

The editor's drawing needs no transform: it runs inside `GameWorld::Draw` with
the world surface as the target, so its handles are already in world
coordinates. The transform is only what turns the mouse position into a point in
the world.

Assumption: "the waypoints" means the waypoints of one state at a time, so each
Path state carries an "Edit in view" radio button and the selected state is the
one drawn and edited. Adding a Path state selects it. A behaviour loaded from
the database starts with nothing selected.

Assumption: a spawn-relative behaviour is drawn and edited around the editor's
spawn Position, which is where the enemy would come in, so what is on screen is
where the enemy would fly.

A click is ignored when ImGui wants the mouse, but a drag already under way
carries on even if the pointer crosses the panel, which is what makes dragging
near the panel usable. This relies on ImGui mouse positions being in window
coordinates, which holds while multi-viewport support is off, as it is by
default in this workspace.

Fixed in passing, inside the function this task rewrote: `DrawPath` left the
offset off its first sample, so the first segment of a preview was drawn from
the wrong place.

**Commits:**
- 69cc3a5 feat(T-005): edit waypoints with the mouse in the game view

**Manual verification:**
- Open the editor (F2), add a Path state, and confirm it is selected for editing
  in the view.
- Click in the game view away from the panel: a waypoint is appended and the
  path redraws.
- Drag a handle: it follows the mouse and the path follows it.
- Right-click a handle: it is deleted.
- Click inside the editor panel: no waypoint is placed.
- Start a drag in the view and pull the mouse over the panel: the drag carries
  on.
- Check the numeric fields still nudge a waypoint, and that the selected one is
  marked in the list.
- Resize the window and confirm a click still lands where it is pointed.

### [done] T-006 Save edited behaviours back to the database

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
- [x] The movement state and its modifiers have serialisers matching their
      loaders.
- [x] Saving loads the whole of `data/behaviour_database.json`, replaces the
      named behaviour, and writes the file back.
- [x] Saving under a new name adds a behaviour rather than replacing one.
- [x] A behaviour saved from the editor reloads on the next run and moves
      identically.

**Out of scope:**
Saving enemy, sprite, weapon or any other database. Preserving the file's
formatting, ordering or comments — a full rewrite is accepted.

**Open questions:**
- Q: Whole file or only the named behaviour? A: Rewrite the whole file. Simple
  and predictable; the cost is that hand-written formatting and ordering are
  lost, which is accepted.

**Notes:**
Each `to_json` writes exactly the fields its `from_json` reads for that type, so
the two cannot drift apart type by type. A label, a duration and a modifier list
are written only when they hold something, which keeps a saved file close to
what a person would have written by hand.

The save reads the whole file, replaces or adds the named behaviour and writes
it back at four-space indent, so ordering and hand formatting are lost, as the
task accepts. It keeps whatever spelling of the `behaviours` category the file
already uses rather than imposing its own.

Checked rather than assumed: a `behaviour_database.json` written in the shape
the serialisers produce, including a Path state with a Wave, a Phase and an Ease
modifier, loads with no warnings or errors. That exercised the modifier loader
from T-003 for the first time. The file was restored afterwards.

Assumption: the database path is a constant in the editor, because the editor is
the only thing that writes one, and the game loads the whole `data/` directory
rather than naming files.

Saving does not update the database the running game holds, so a behaviour saved
under a new name appears in the Behaviour combo on the next run. The panel says
so.

**Commits:**
- ad65d0e feat(T-006): save edited behaviours back to the database

**Manual verification:**
- Open the editor (F2), load `basic`, move a waypoint, and press Save. The file
  `data/behaviour_database.json` holds the moved waypoint.
- Restart and confirm `basic` spawns with the edited path.
- Save under a name that is not in the database. The file gains a behaviour and
  keeps the others. Restart and confirm the new name is in the Behaviour combo.
- Save a behaviour with a Wave, a Phase and an Ease modifier, restart, load it
  and confirm it moves as it did before the restart.

### [done] T-007 Preview a path with a scrubbed ghost

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
- [x] A ghost can be scrubbed along the current path with a slider.
- [x] The ghost can play back at the state's real speed, with play and pause.
- [x] The preview shows direction along the path.
- [x] The preview shows time markers.
- [x] The ghost reflects modifiers, so a wave shows as a wave.

**Out of scope:**
Previewing a whole behaviour across several states. This is one state's path.

**Open questions:**
- Q: Scrub only, or playback too? A: Both. Playback is what makes the preview
  worth having; scrubbing is for inspecting a moment.

**Notes:**
The preview samples through `SamplePathState`, the same function the path
handler uses, so the ghost, the arrows and the markers all reflect the
modifiers: a wave shows as a wave, and the direction arrows follow the flown
path rather than the line underneath it. The flown path is drawn over the plain
geometry, so the two can be compared.

Playback advances the scrub by `speed * delta / length`, which is the state's
real speed, and wraps back to the start so a shape can be watched over and over.
The panel shows where the ghost is in seconds, and how long the whole path takes.

Assumption: the time markers are ticks across the path, one for each second and
a longer one every fifth, rather than numbers. The editor draws into the world
surface where it has no font, and a tick is readable at the size the path is
drawn at.

The arc-length table is rebuilt every frame, because the path is being edited
every frame. It is a thousand samples of a two-dimensional curve, which does not
show in the frame times.

**Commits:**
- b82bb56 feat(T-007): preview a path with a scrubbed ghost

**Manual verification:**
- Open the editor (F2) and select a Path state with two or more waypoints.
- Drag the Scrub slider: the ghost moves along the path and its heading follows
  the path.
- Press Play: the ghost runs at the state's speed and loops. Pause stops it
  where it is.
- Confirm the arrows point the way the entity travels, and that the ticks across
  the path are one second apart, with a longer one every fifth.
- Add a Wave modifier: the flown path, the ghost, the arrows and the ticks all
  follow the wave, while the plain path still shows the line underneath it.
- Spawn the same behaviour and confirm the enemy flies where the ghost went.

### [done] T-015 Remove the dead spawner path

**Review:** reviewed 2026-09-20

**Depends on:** none

**Goal:**
`SystemEnemySpawner::CreateSpawner` (`src/system_enemy_spawner.cpp:198`) has no
callers. Nothing ever constructs a `ComponentEnemySpawner`, so
`SystemEnemySpawner::Update` iterates an empty view every frame and the whole
`SpawnerData` path is unreachable: `SpawnerType`, `offset_step`,
`formation_positions`, `group_count`, `group_delay`, `tick_offset`, `kill_type`
and `data/spawner_database.json`. The only live entry point on that class is
`SystemEnemySpawner::SpawnEnemy`, called from `SpawnMapEnemy`
(`src/system_map.cpp:151`) and from the behaviour editor. All enemy spawning goes
through map objects.

`BehaviourParameterList` and its `adl_serializer` (`src/gamedata_spawner.h:28`)
sit inside that dead path and are dead twice over: nothing reads them even there,
and the `behaviour_parameters` blocks in `data/spawner_database.json` use `speed`,
`frequency` and `amplitude` keys naming the `Line`, `Wave` and `Spline` behaviour
types that T-002 replaced with waypoint paths. The spawners in that file also
reference behaviours (`Spline`, `Straight`, `Wave`) that are not in
`data/behaviour_database.json`.

That parameter list is the generic string-to-variant approach to per-spawn
parameters, and T-012 deliberately does not revive it: it is stringly typed, gives
the editor nothing to validate or preview, and would need a resolve pass over every
field in the data. A typed struct that grows a field when one is needed is the
better trade. Removing the dead path keeps it from being mistaken for the intended
design, and clears the area T-012 works in.

This task runs before T-011 to T-014 so that they are not written against code that
never runs.

**Requirements:**
- [x] `SystemEnemySpawner::CreateSpawner`, `SystemEnemySpawner::Update` and
      `ComponentEnemySpawner` are removed, along with the `SystemEnemySpawner`
      entry in `GameWorld::Update` (`src/game_world.cpp:58`).
- [x] `SystemEnemySpawner::SpawnEnemy` stays, with its callers in `SpawnMapEnemy`
      and the behaviour editor unchanged.
- [x] `SpawnerData`, `SpawnerType`, `EnemyKillType`, `BehaviourParameterList` and
      its `adl_serializer` specialisation are removed from
      `src/gamedata_spawner.h`, and the file goes if nothing is left in it.
- [x] `GameDataCategory::Spawners`, `Database<SpawnerData> m_spawnerDatabase`
      (`src/gamedata.h:86`) and the `SpawnerData` branches of the `if constexpr`
      chains in `src/gamedata.h` are removed, as is the `SpawnerData` forward
      declaration in `src/gamedata_fwd.h`.
- [x] `data/spawner_database.json` is deleted.
- [x] The **Compatibility** section of `docs/task-profile.md` no longer lists
      `data/spawner_database.json`, in the same commit as the deletion.
- [x] `EnemyKillType` is only removed if nothing live uses it. `SpawnMapEnemy`
      applies `ComponentBounds` directly, so check before deleting.
- [x] The game still plays the star map exactly as it does today. Nothing live
      changes behaviour.

**Out of scope:**
Anything to do with `SpawnMapEnemy`, `ComponentBounds`, `SystemGroup` or the
behaviour editor. This task only deletes what is unreachable.

**Open questions:**
- Q: The spawners in `data/spawner_database.json` reference behaviours that no
  longer exist. Is that file still live test data worth repairing, or is it dead
  alongside the parameter blocks? A: It is dead. The whole spawner path is
  unreachable, so the file is deleted with it and the task profile's
  Compatibility list is updated to match.

**Notes:**
`EnemyKillType` was checked before deleting: only the removed `Update` used it.
`SpawnMapEnemy` builds its own `ComponentBounds`, so nothing live lost anything.
Removing `gamedata_spawner.h` and the `system_enemy_spawner.h` include of it took
transitive includes away from three translation units, so `system_behaviour.h` and
`gamedata_weapon.h` were added to `src/game_world.cpp`, `gamedata_enemy.h` to
`src/gamedata.cpp`, and `system_behaviour.h` to `src/system_enemy_spawner.h`. The
includes in `src/system_enemy_spawner.cpp` that only the deleted code needed
(`system_group.h`, `system_lifetime.h`, `system_world_bounds.h`, `game_world.h`,
`spdlog`) went with it.

**Commits:**
- `b893c7d` refactor(T-015): remove the unreachable spawner path

**Manual verification:**
1. Play the star map and confirm enemies spawn, fly and die exactly as before.
2. Open the behaviour editor with F2 and spawn an enemy from it; spawning still
   works.

### [done] T-011 Anchor behaviour paths on their first waypoint and rotate them at spawn

**Review:** reviewed 2026-09-20

**Depends on:** none

**Goal:**
Enemy behaviours can only be varied per spawn by speed. A path that travels
straight down the screen can only be made to travel at an angle by authoring a
second behaviour with an angled path, and a third for the next angle, and so on.
Instead the spawn should be able to supply a rotation, and the behaviour builds
its path from that.

`ComponentBehaviour::m_data` (`src/system_behaviour.h:22`) is already a
`BehaviourData` held by value, so every spawned enemy has its own copy of the
behaviour, and `system_map.cpp:108` already exploits that to overwrite the speed
of every state. That is the seam: rotation is applied as a transform on the copy
at spawn time, not as a parameter the path sampler reads. Everything downstream
is derived from `waypoints` (the arc-length table, the `Wave` modifier's normal
via `PathTangentAtDistance`, the `entry` approach's target), so rotating the
waypoints carries all of it with no further work.

All paths operate in a space defined by their first point. The first waypoint of
the first `Path` state in the behaviour is the anchor, and it is the pivot for
every state in that behaviour, in both spaces. It has to be behaviour-wide rather
than per-state, or the behaviour tears apart at each state boundary because a
later state would rotate about a point the state before it no longer ends at.

For `SpawnRelative`, the anchor should additionally be moved to wherever the enemy
spawned, regardless of where the path was authored. Without this, a spawn-relative
path has to be authored with its first point at the spawn point, which is normally
at the edge of the screen, making paths that travel left or up impossible to see
while authoring.

Doing both as a pre-pass on the copy means `src/system_behaviour.cpp` does not
change: after translating the anchor to `(0,0)`, the existing
`m_offset = behaviour.m_origin` puts the first waypoint exactly on the spawn
point.

Two consequences are expected and correct. A spawn-relative behaviour's *first*
path state now begins exactly where the enemy is, so `SetUpEntry`
(`src/system_behaviour.cpp:177`) returns early and no approach transition runs for
it. Later path states are translated by the same anchor, so they do not start on
the enemy and their `entry` rule still runs.

**Requirements:**
- [x] A free function transforms a `BehaviourData` in place, given a rotation, and
      is called at spawn time on the per-entity copy.
- [x] The anchor is the first waypoint of the first `Path` state in the behaviour,
      and is the pivot for every state in the behaviour.
- [x] A behaviour with no `Path` state has no anchor, and the transform is a no-op.
- [x] In both `BehaviourSpace::World` and `BehaviourSpace::SpawnRelative`, every
      waypoint of every `Path` state is rotated about the anchor.
- [x] In `BehaviourSpace::SpawnRelative` only, every waypoint is then translated by
      the negated anchor, so the first waypoint of the first path state lands on
      the spawn point.
- [x] The rotation is given in degrees, positive clockwise on screen, and converted
      to radians at the point of use.
- [x] `Target` states are not rotated and not translated. Their `point` stays
      absolute world.
- [x] `src/system_behaviour.cpp` is unchanged.
- [x] `data/behaviour_database.json` keeps working unedited. `basic` is anchored at
      `[0, 0]`, so its translation is zero and it flies as it does today.

**Out of scope:**
Resolving the inconsistency where `BehaviourHandlerTarget` uses `state.point` as an
absolute world position and ignores `space` and `m_origin` entirely. That is a
known defect, recorded in `docs/behaviour-paths-and-editor.md`, and it should not
be half-resolved as a side effect of this task.

Exposing the rotation in any data file or UI. That is T-012 and T-013. This task
adds the function and calls it with a rotation of zero, which must leave every
existing behaviour flying exactly as it does today.

**Open questions:**
- Q: What are the units and sign convention for the rotation: degrees or radians,
  and is a positive value clockwise or anticlockwise on screen? A: Degrees,
  positive clockwise, in data and in the editor, matching what Tiled shows a
  designer. Converted to radians at the point of use, which agrees with
  `moth::tilemap::MapObject::rotation` (radians, clockwise, converted from Tiled's
  degrees on load, `moth/tilemap/tile_map.h:62`).

**Notes:**
`TransformBehaviour` and `BehaviourAnchor` live in the new header
`src/behaviour_transform.h`. It is header-only, so `CMakeLists.txt` does not
change; `SOURCES` lists only `.cpp` files.
The anchor is the first waypoint of the first path state *that has waypoints*, so
an empty path state part way through authoring does not swallow the anchor.
Rotation uses `moth::gfx::Rotate2D` with `moth::gfx::DegToRad`. In the game's
y-down space that matrix turns clockwise on screen, which is the convention the
task asks for.
Assumption: the call site is `SpawnMapEnemy` only, the game's spawn path. The
behaviour editor's own spawn gets the call in T-013, together with the canvas and
rotation control that go with it.

**Commits:**
- `a4a7923` feat(T-011): anchor behaviour paths and rotate them at spawn

**Manual verification:**
1. Play the star map. Enemies using `basic` fly exactly as they did before, since
   its path is anchored at `[0, 0]`.
2. Spawn-relative behaviours authored with a first waypoint away from the origin
   now start on the spawn point rather than offset from it.

### [done] T-012 Let map spawns supply a behaviour rotation

**Review:** reviewed 2026-09-20

**Depends on:** T-011

**Goal:**
Give the rotation from T-011 a home in the map data, so a designer sets an angle on
the spawn rather than authoring another behaviour.

`SpawnMapEnemy` (`src/system_map.cpp:73`) is the only live place that builds a
per-entity behaviour copy for the game. It copies the behaviour and then overwrites
the speed of every state from a `speed` Tiled property (`src/system_map.cpp:108`).
That hardcoded override should be generalised into one typed override that also
carries the rotation, so the two are read and applied together rather than one
being special-cased.

A rotation that steps per spawn index gives a fan out of a formation for free, and
matches an idiom the behaviour system already has in the `Phase` modifier's
`cycles` per formation index. The spawn loop already tracks that index as
`phaseIndex` (`src/system_map.cpp:154`).

The rotation comes from a custom Tiled property, not from the object's native
`rotation`, because T-010 uses the native field to rotate formation offsets. The
two are independent controls: rotating the object in Tiled angles the arrangement
of the group, and the custom property angles the path each member flies.

**Requirements:**
- [x] A typed override struct carries at least the speed and the rotation in
      degrees. It is not a generic string-to-variant parameter map.
- [x] One function applies the override to a `BehaviourData` copy, using T-011's
      rotation function rather than its own copy of the maths.
- [x] The existing hardcoded `speed` override in `SpawnMapEnemy` is replaced by
      that function, and the `speed` Tiled property keeps working as it does today,
      including being read as either an int or a float.
- [x] A map spawn sets the rotation through a custom property named
      `behaviour_rotation`, read as either an int or a float.
- [x] A second custom property steps the rotation per spawn index, so a formation
      fans out. It is an angle added per index, not a total spread divided across
      the group.
- [x] The properties are declared on the `EnemySpawn` class in
      `tiled_project/tincan.tiled-project`, so they can be set in Tiled.
- [x] The object's native `rotation` is not read by this task.
- [x] Existing maps keep working without being edited. A spawn with neither
      property set behaves exactly as it does today.

**Out of scope:**
The dead spawner path (`SpawnerData`, `ComponentEnemySpawner`,
`data/spawner_database.json`). T-015 deletes it, and this task must not add the
override to it.

The object's native `rotation` field, which belongs to T-010.

**Open questions:**
- Q: Does a map spawn's behaviour rotation come from the Tiled object's own
  `rotation` (`moth::tilemap::MapObject::rotation`, which `SpawnMapEnemy` ignores
  today) or from a separate custom property? A: A separate custom property,
  `behaviour_rotation`. T-010 keeps the native `rotation` for formation offsets, so
  the arrangement of a group and the path its members fly are set independently.
- Q: Is the rotation step an angle per index, or a total spread divided across the
  formation? A: An angle added per index.

**Notes:**
`BehaviourOverride` and `ApplyBehaviourOverride` sit beside `TransformBehaviour`
in `src/behaviour_transform.h`, so the override and the maths it uses stay
together. `speed` is a `std::optional<float>`, so a spawn without the property
leaves every state at its authored speed, as before.
The step property is named `behaviour_rotation_step`, an angle added per spawn
index. Because the rotation now varies along the formation, each member gets its
own copy of the behaviour inside the spawn loop rather than one copy shared by
the whole spawn. The index used is the same `phaseIndex` the `Phase` modifier
uses.
Both new properties are read through `GetFloatProperty`, which already accepts an
int or a float, and both default to 0, so existing maps spawn exactly as before.

**Commits:**
- `3b88d0e` feat(T-012): let map spawns rotate the behaviour they use

**Manual verification:**
1. Play the star map: spawns with neither new property set fly as they did
   before, and the `speed` property still overrides the behaviour's speed.
2. In Tiled, set `behaviour_rotation` to 45 on an `EnemySpawn` and confirm the
   enemy's path is turned clockwise by that much.
3. Set `count` to 5 and `behaviour_rotation_step` to 10, and confirm the group
   fans out, each member turned 10 degrees further than the one before.
4. Confirm both properties appear on the `EnemySpawn` class in Tiled.

### [done] T-013 Make the behaviour editor canvas an authoring space

**Review:** reviewed 2026-09-20

**Depends on:** T-011

**Goal:**
Once T-011 re-anchors a spawn-relative path onto the spawn point, the editor no
longer has to draw the path at the spawn position for it to be authored correctly.
The canvas becomes pure authoring space: waypoints draw at their literal
coordinates with their handles, anywhere on screen, and the re-anchoring happens
when the enemy spawns.

This changes what `Position (px)` (`src/system_behaviour_editor.cpp:252`) means.
For a spawn-relative behaviour the drawn path and the spawned enemy now differ by a
translation: the same shape, in a different place on screen. Defaulting
`Position (px)` to the anchor makes the spawned enemy fly along the line that was
just drawn, and letting the user type a value means "fly this shape from somewhere
else". For a `World` behaviour `Position (px)` keeps its current meaning and stays
useful, because the path is absolute and the position is where the enemy comes from
before its `entry` approach joins the path.

The editor also gets a rotation on the spawn controls. It does not rotate anything
that is drawn; it feeds the spawn, so a rotated enemy can be watched flying. That
makes the editor exercise T-011's transform before the game does.

**Requirements:**
- [x] For a spawn-relative behaviour, waypoints and their handles draw at their
      literal authored coordinates, and mouse picking, placement and dragging work
      in that same space.
- [x] A spawn-relative path can be authored anywhere on screen, including travelling
      left or up from where it starts.
- [x] `Position (px)` follows the anchor until the user types a value into it, and
      stays where the user put it from then on.
- [x] Loading a different behaviour, or clearing the one being edited, makes
      `Position (px)` follow the anchor again.
- [x] For a `World` behaviour, `Position (px)` keeps its current meaning.
- [x] A rotation control sits under the spawn controls, above the separator at
      `src/system_behaviour_editor.cpp:269`, as a slider in degrees that also
      accepts typed input. `ImGui::SliderFloat` gives both, because ctrl+click on a
      slider turns it into a text input.
- [x] The rotation applies to the spawned enemy only. It does not change the drawn
      path, the handles or the scrubbed ghost preview.
- [x] The rotation uses T-011's function, not its own copy of the maths.

**Out of scope:**
Guides. Loading another behaviour as a read-only reference is T-014.

**Open questions:**
- Q: Does `Position (px)` follow the anchor at all times, or only until the user
  edits it by hand? A: It follows until the user types a value, then stays put. It
  starts following again when a different behaviour is loaded.

**Notes:**
The canvas origin is now always `{0, 0}`, which is what makes the drawing, the
mouse picking, the placement and the dragging all work in authored coordinates.
For a `World` behaviour that is what it already was, so nothing changed there.
`Position (px)` follows `BehaviourAnchor` while the behaviour is spawn-relative
and the field has not been typed into. It starts following again when another
behaviour is loaded from the combo, and also when the behaviour has no path state
to take an anchor from, which covers clearing the one being edited.
The rotation control is `Spawn rotation (deg, clockwise)`, a `SliderFloat` over
-180 to 180. It is applied with `TransformBehaviour` to a copy made when Spawn is
pressed, so nothing drawn is affected.

**Commits:**
- `b2b6478` feat(T-013): make the behaviour editor canvas an authoring space

**Manual verification:**
1. Open the editor with F2, load `basic`, and confirm the path draws where its
   waypoints say rather than at the spawn position.
2. Author a spawn-relative path that travels left and up from its first point and
   confirm it can be drawn and dragged anywhere on screen.
3. Drag the first waypoint and confirm `Position (px)` follows it; type a value
   into `Position (px)` and confirm it stops following.
4. Load a different behaviour and confirm `Position (px)` follows the anchor
   again.
5. Switch Space to `World` and confirm `Position (px)` no longer follows, and the
   path still draws at its literal coordinates.
6. Set the spawn rotation to 45 and press Spawn: the enemy flies a turned path
   while the drawn path, its handles and the scrubbed ghost stay where they are.

### [done] T-014 Load behaviours as read-only guides in the editor

**Review:** reviewed 2026-09-20

**Depends on:** T-013

**Goal:**
Instead of rotating the path being edited to preview an angle, let the editor load
other behaviours as guides. A guide draws its path as a reference and gets its own
rotation and its own anchor, but is read-only. A designer can then put paths side
by side and see how they interact: the same behaviour loaded twice at different
rotations shows a fan, and two different behaviours show whether two spawners'
paths collide.

Read-only falls out of the existing structure. The mouse code
(`src/system_behaviour_editor.cpp:367`) only ever touches `editedState->waypoints`,
so guides held beside `staticBehaviourData` are invisible to `FindWaypoint`,
placement and dragging without needing a guard.

A guide shows the version saved in the database, not unsaved edits, so loading the
behaviour being edited as its own guide will not preview live work.

**Requirements:**
- [x] The editor holds a list of guides, each with a behaviour name, a rotation in
      degrees and an anchor position.
- [x] A guide can be added and removed, with no limit on how many are loaded.
- [x] Guides are held in editor state like the rest of the editor's statics. They
      are not saved anywhere and are gone when the app closes.
- [x] A guide's path draws in a colour distinct from both the edited path and
      `ReferencePathColor()`, which already means "another state of the behaviour
      being edited".
- [x] A guide is read-only: it has no handles, and it is never hit by waypoint
      picking, placement or dragging.
- [x] A guide's rotation uses T-011's function.
- [x] Guides are editor state only. They are never written into `BehaviourData`,
      `SaveBehaviour` or `data/behaviour_database.json`.

**Open questions:**
- Q: Is there a limit on how many guides can be loaded at once? A: No limit.
- Q: Do guides persist across restarts of the app, or are they set up again each
  time the editor is opened? A: They are not persisted. The editor saves none of
  its own state today, and adding a format for it is not worth the task.

**Notes:**
The guides panel sits at the bottom of the editor window: a combo of database
behaviours, an `Add guide` button, and per guide a rotation slider, an anchor
field and a `Remove` button. `staticGuides` is a `std::vector<BehaviourGuide>`
beside the other editor statics, so nothing about a guide is persisted.
Guides draw first, so the behaviour being edited sits on top of them. Each is
copied from the database, passed through `TransformBehaviour`, and drawn with
`DrawFlownPath` in the new `GuidePathColor()` (purple), distinct from the edited
path (yellow), `ReferencePathColor()` (grey) and `EntryPathColor()` (teal).
Assumption: a guide's anchor places it the way a spawn position would, so it is
applied to a spawn-relative guide and ignored for a `World` guide, whose path is
absolute. That matches how the game places the two spaces.
A guide naming a behaviour that is not in the database is skipped rather than
logged, because the combo only offers names the database has.

**Commits:**
- `f442cd4` feat(T-014): load behaviours as read-only guides in the editor

**Manual verification:**
1. Open the editor with F2, add `basic` as a guide twice, and give the two
   different rotations: the two purple paths show the fan.
2. Confirm a guide has no handles, and that clicking or dragging on a guide's
   line adds or moves a waypoint of the edited path instead of touching the
   guide.
3. Remove a guide and confirm it stops being drawn, and that the others stay.
4. Save the behaviour being edited and confirm `data/behaviour_database.json`
   holds no trace of the guides.

### [done] T-010 Describe spawn formations in the game data

**Review:** reviewed 2026-09-20

**Depends on:** none

**Goal:**
The formations a map spawn can use are the four hardcoded in the if-chain in
`GetFormationOffsets` (`src/system_map.cpp:38`): line, column, v and grid. There
is no way to stagger a formation on an angle, and no way to make a shape like a
diamond or a star without writing another branch and hand-editing the
`formation` enum in `tiled_project/tincan.tiled-project`.

Move formations into the game data, the same way enemies, behaviours and
pickups are already done: a `FormationData` type, a
`GameDataCategory::Formations` category and a `data/formation_database.json`.
A formation entry is an ordered list of slot offsets in unit space that
describes the *largest* version of the shape. The spawn's `count` property
fills the slots one by one from the front, so a single entry serves every count
up to its size, and whatever shape is authored progressively fills out as the
count grows.

On top of that, the offsets are transformed before use by a rotation and a
scale. The rotation is the Tiled object's own `rotation`, which
`moth::tilemap::MapObject` already carries and `SpawnMapEnemy` currently
ignores, so a formation is staggered on an angle by rotating the object in
Tiled and the angle is visible while placing it. The scale lets larger enemies
reuse the same shapes without their slots overlapping.

`SpawnerData::formation_positions` was the same idea in the older spawner path.
T-015 deletes that path, so it is not there to copy from if this task runs after
it. The pattern to follow is the other game data categories in `src/gamedata.h`.

**Requirements:**
- [x] `FormationData` and `GameDataCategory::Formations` follow the pattern of
      the other game data categories, loaded from `data/formation_database.json`.
- [x] A formation entry is an ordered list of slot offsets in unit space,
      describing the largest version of the shape.
- [x] The spawn's `count` fills the slots one by one from the front of the list.
- [x] Slots are placed where they are authored. The used subset is not
      re-centred on the spawn point when the count is under the full size.
- [x] A `count` larger than the slot list spawns the slots that exist and logs a
      warning naming the formation, the requested count and the slot count.
- [x] `GetFormationOffsets` looks the formation name up in the database first
      and falls back to the built-in line, column, v and grid generators, which
      stay because they are not bounded by a count.
- [x] Existing maps keep working without being edited.
- [x] The offsets are rotated by the Tiled object's `rotation`
      (`moth::tilemap::MapObject::rotation`, radians clockwise), which
      `SpawnMapEnemy` currently ignores.
- [x] A database formation is sized by a per-axis `scale` property on the spawn
      object, and ignores `spacing`. The built-in line, column, v and grid keep
      using `spacing` exactly as they do today and ignore `scale`.
- [x] `scale` defaults to 1 on both axes, so a formation with no scale set is
      used at the size it was authored.
- [x] The `formation` enum is no longer maintained by hand in
      `tiled_project/tincan.tiled-project`: it is synced by
      `tiled_project/extensions/game_data_enums.js` with
      `extra: ["line", "column", "v", "grid"]` for the built-ins.
- [x] `data/formation_database.json` ships with a diamond and a star.

**Out of scope:**
The behaviour rotation from T-012, which uses its own `behaviour_rotation`
property. This task owns the object's native `rotation` and must not read or
change `behaviour_rotation`.

Changing what `spacing` means for the four built-in formations. Maps already set
it and must keep working.

**Open questions:**
- Q: How does the new scale relate to the existing `spacing` property on the
  spawn, which already multiplies the unit offsets? Is scale a second
  multiplier, or does it replace `spacing` for data formations? A: It replaces
  it. A database formation is authored as a shape and uses `scale` only; the
  built-in generators are built from a step and keep using `spacing`. The two
  never multiply together.
- Q: Is the scale uniform, or per-axis? A: Per-axis, so a shape can be stretched
  across the screen without being stretched down it. Formation offsets are
  already `FloatVec2`, so this costs nothing.
- Q: Where does the scale come from: a property on the spawn object, the enemy
  data (so a large enemy scales its formation automatically), or both? A: A
  property on the spawn object, like `count` and `spacing` already are. Enemy
  data does not take part.

**Notes:**
`FormationData` is in the new `src/gamedata_formation.h` and follows the other
categories. `Database` gained a `Find`, a lookup that does not log when a name is
missing, because `GetFormationOffsets` asks the database first and a built-in
name is not an error. `Get` now calls `Find` and logs, so nothing else changes.
Assumption: the per-axis scale is two float properties, `scale_x` and `scale_y`,
both defaulting to 1, rather than a Tiled class property, so it is read with the
same `GetFloatProperty` as `spacing` and shows as two plain fields in Tiled.
The rotation is applied to the finished offsets inside `GetFormationOffsets`, so
it covers both the database shapes and the built-in generators. No `EnemySpawn`
object in either map is rotated today, so no map moves.
`data/formation_database.json` ships `diamond` and `star`. The `formation` enum in
`tiled_project/tincan.tiled-project` is left as the four built-ins in the file;
the extension rewrites it with the database shapes when Tiled next opens the
project.

**Commits:**
- `8424b49` feat(T-010): describe spawn formations in the game data

**Manual verification:**
1. Play the star map: every existing spawn keeps its formation, spacing and
   shape.
2. Open the project in Tiled and confirm the `formation` enum now also offers
   `diamond` and `star`, and that `scale_x` and `scale_y` appear on `EnemySpawn`.
3. Set a spawn to `diamond` with `count` 9 and `scale_x`/`scale_y` around 40, and
   confirm the shape appears and that `spacing` has no effect on it.
4. Drop the count to 4 and confirm the first four slots are used where they were
   authored, not re-centred.
5. Set the count to 20 and confirm the log warns, naming the formation, the
   count and the slot count, and that only the authored slots spawn.
6. Rotate the spawn object in Tiled and confirm the whole arrangement turns with
   it, for a database shape and for `line`.

### [done] T-016 Guard the pickup system against a missing player

**Review:** reviewed 2026-09-20

**Depends on:** none

**Goal:**
`SystemPickup::Update` logs `[error] Unable to locate player` and then
dereferences a null `playerPosition` in the pickup loop that follows
(`src/system_pickup.cpp:132`-`152`). It is reachable once the player entity is
gone. Found by the T-008 launch check during the 2026-09-19 session, recorded
under `## Discovered`, and left unfixed because it was outside every task in that
session.

**Requirements:**
- [x] The pickup loop does not run when there is no player, and nothing
      dereferences the null `playerPosition`.
- [x] Pickups behave exactly as they do today while a player exists.
- [x] The error log is not made noisier. It should not log once per frame for the
      rest of the run.

**Out of scope:**
Any other null-player path in another system. This task fixes the one that was
found.

**Notes:**
The `[error] Unable to locate player` line is gone rather than rate-limited. A
missing player is the normal state between the player dying and the next one, so
there is nothing to report, and holding a "already warned" flag in a system
static would be state the project keeps in components. `playerBody` is filled by
the same view as `playerPosition`, so the one check covers both.
The launch check was intermittently failing on this before the fix: a run in
which the player died inside the five seconds filled the log with the error and
failed the check.

**Commits:**
- `778bde3` fix(T-016): skip the pickup sweep when there is no player

**Manual verification:**
1. Play until the player dies with pickups on screen: the game keeps running and
   the log holds no `Unable to locate player` lines.
2. While a player is alive, fly over a weapon pickup and a passive pickup and
   confirm both are still collected.

### [done] T-017 Document the path entry rule

**Review:** reviewed 2026-09-20

**Depends on:** none

**Goal:**
Commit `d6965d5` added an `entry` field to a path state, saying how the state
travels to its own start: `Snap`, `Linear` or `Spline`. It runs before the
state's duration starts counting, and it also covers the jump from the spawn
point into the first state. It changed the behaviour data format.

It was worked through directly with the user rather than taken through planning,
so nothing describes it apart from the commit and the 2026-09-19 session log,
which flags this as something for planning to decide. `docs/behaviour-paths-and-editor.md`
is the document that describes the behaviour data format and the editor, and it
does not mention `entry` at all.

**Requirements:**
- [x] `docs/behaviour-paths-and-editor.md` describes the `entry` field: its three
      values, that it runs before the state's duration starts counting, and that
      it covers the spawn into the first state.
- [x] The document says how `entry` interacts with T-011's anchoring, once T-011
      is done: a spawn-relative behaviour's first path state begins where the
      enemy already is, so its entry rule has nothing to do.
- [x] Prose is wrapped at about 85 columns, as the workspace `CLAUDE.md` asks.
- [x] No code changes.

**Out of scope:**
Changing how `entry` works. This task only writes down what it already does.

**Notes:**
The description is a new `### Entering a state` subsection under `## The model`,
between `### A path state` and `### Modifiers`, which is where the rest of a path
state's fields are described. It covers the three values, the approach running on
its own clock before the state's duration, the spawn into the first state, and
the anchoring from T-011, which is done in this session.

**Commits:**
- `facd1ea` docs(T-017): describe the path entry rule

**Manual verification:**
1. Read `docs/behaviour-paths-and-editor.md`, section "Entering a state", and
   confirm it matches how `entry` behaves in the editor.

### [done] T-019 Share the formation offsets and the single-sprite draw

**Review:** reviewed 2026-09-20

**Depends on:** none

**Goal:**
Preparation for the formation editor (T-020, T-021). Both pieces of code it needs
are private to a `.cpp` today.

`GetFormationOffsets` (`src/system_map.cpp:38`) turns a formation name, a count, a
spacing, a per-axis scale and a rotation into the offsets of each member, asking
`data/formation_database.json` first and falling back to the built-in `line`,
`column`, `v` and `grid` generators. It sits in the anonymous namespace of
`src/system_map.cpp`, so the editor cannot call it, and a second copy of it in the
editor would let the preview drift away from what the game spawns.

The per-sprite draw sits inside the batch loop in `SystemDrawable::Draw`
(`src/system_drawable.cpp:155`), which reads a `ComponentSprite` off an entity,
resolves the animated case to a frame and a pivot, and draws it. T-021 draws the
selected enemy's sprite at each filled slot with no entity behind it, and should
get the same picture the game draws rather than its own call to `DrawImage`.

Nothing changes about what the game does. This task only moves code so that two
callers can share it.

**Requirements:**
- [x] `GetFormationOffsets` moves to a shared header, `src/formation_offsets.h`,
      with its current signature and behaviour unchanged.
- [x] `src/system_map.cpp` calls it from there, and spawning is unchanged.
- [x] The single-sprite draw is exposed as a function taking the graphics, a
      `ComponentSprite`, a position and an angle, and `SystemDrawable::Draw` uses
      it for each entry of its batch, so there is one draw path.
- [x] The batch, its z-order sort, the camera offset and the blend mode stay in
      `SystemDrawable::Draw`. Only the draw of one sprite moves.
- [x] Any new `.cpp` is added to `SOURCES` in `CMakeLists.txt`.
- [x] The star map plays and draws exactly as it does today.

**Out of scope:**
Any editor change, and any change to how formations are chosen or sprites are
animated. This is a move, not a redesign.

**Open questions:**
- Q: Where does the shared formation helper live? A: `src/formation_offsets.h`,
  beside the other small shared headers. It takes `GameData const&` as its first
  argument, as it does now.

**Notes:**
`GetFormationOffsets` is an `inline` function in the new header, so no `.cpp`
was added and `SOURCES` did not change. `DrawSprite` now resolves the animated
frame and does the whole-pixel snap itself; the batch stores the unsnapped
world position and the entity angle, which draws the same picture as before.
`src/system_map.cpp` no longer includes `gamedata_formation.h`, which it needed
only for the moved function.

**Commits:**
- a6aee1f refactor(T-019): share the formation offsets and the single-sprite draw

**Manual verification:**
- Play the star map and check that enemies spawn in the same formations and at
  the same places as before, and that sprites, flashes and blend modes look
  unchanged.

### [done] T-020 Add a formation editor window

**Review:** reviewed 2026-09-20

**Depends on:** T-019

**Goal:**
Formations are authored by hand in `data/formation_database.json` and the only way
to see one is to set it on a map spawn and play the map. Give them an editor, the
way behaviours have one.

F2 becomes editor mode rather than "the behaviour editor is open". It shows both
the existing Behaviour Editor window and a new Formation Editor window. They are
separate ImGui windows so they can be docked together, and the most recently
focused of the two owns the world view: its overlay draws there and the mouse
edits its own shape. Without that rule two sets of handles would share the screen
and fight over the same clicks.

A formation entry is an ordered list of slot offsets in unit space describing the
largest version of the shape, which a spawn's `count` fills from the front
(`FormationData`, `src/gamedata_formation.h`). The editor authors that list and
previews it at a count, a per-axis scale and a rotation: the three things a map
spawn applies. The preview goes through the shared `GetFormationOffsets` from
T-019, so what is drawn is what a spawn would place.

**Requirements:**
- [x] F2 toggles editor mode, which shows both the Behaviour Editor and the new
      Formation Editor as separate ImGui windows.
- [x] Editor mode pauses the map update exactly as the behaviour editor does
      today (`src/game_world.cpp:53`).
- [x] The most recently focused of the two windows owns the world view: its
      overlay is drawn there and the mouse edits its shape. The Behaviour Editor
      is the active one when editor mode opens.
- [x] Clicking in the world view does not change which window is active.
- [x] The behaviour editor keeps its current drawing and mouse editing, including
      its guides, while it is the active window.
- [x] The Formation Editor has a combo over the formations the game knows: the
      entries of `data/formation_database.json` and the built-in `line`, `column`,
      `v` and `grid`.
- [x] A built-in formation can be selected and previewed, using `spacing` rather
      than scale, but has no slots to edit and cannot be saved.
- [x] A database formation's slots can be created and edited: click in the world
      view to add a slot, drag to move one, right-click to remove one, at the
      preview scale, stored in unit space.
- [x] The panel lists the slots as editable unit-space number pairs with a remove
      button and an add button, as the waypoint list does.
- [x] The preview controls are a count, a per-axis scale and a rotation in
      degrees, and the preview is built with `GetFormationOffsets` from T-019.
- [x] Slots within the count are drawn differently from the slots beyond it, so a
      shape can be judged at the count it will be spawned with.
- [x] A `Save as` name and a Save button write the formation back to
      `data/formation_database.json`, replacing an entry of that name or adding a
      new one, mirroring `SaveBehaviour`
      (`src/system_behaviour_editor.cpp:38`). Save is disabled for a built-in.
- [x] Any new `.cpp` is added to `SOURCES` in `CMakeLists.txt`.

**Out of scope:**
Drawing the enemy's sprite at a filled slot, which is T-021. This task draws slot
markers only.

Changing how formations are spawned, what the built-in generators produce, or the
`FormationData` schema.

**Open questions:**
- Q: Where does the formation editor live: its own panel behind another key, or
  part of the behaviour editor? A: Its own ImGui window, shown by the same F2,
  which becomes editor mode. Separate windows so the two can be docked together.
- Q: Does it save to `data/formation_database.json`? A: Yes, the way the behaviour
  editor saves behaviours.
- Q: How is the active window decided? A: The most recently focused one, which
  keeps the world view until the other window is focused.
- Q: Can the built-in formations be edited? A: No. They can be selected and
  previewed, but they are generated from a count and a spacing, so they have no
  slots and cannot be saved.

**Notes:**
Assumptions made: the preview is centred on the middle of the world, because a
formation has no spawn point of its own in the editor; the default preview scale
is 32 px per unit; the world view owner changes only while one of the two windows
reports itself focused, so a click on the world, which focuses neither, leaves it
alone.

`GetFormationOffsets` gained an overload taking a `FormationData` rather than a
name. The editor previews the entry it is editing, which is not in the database
until it is saved, so a name lookup would show the saved shape instead of the
edit. The database branch of the name version now calls the new overload, so
there is still one code path; the built-in generators and the spawn path are
unchanged. `FormationData` gained a `to_json` so the editor can write it back;
its schema is unchanged.

**Commits:**
- 28e8f74 feat(T-020): add a formation editor window

**Manual verification:**
- Press F2 and check that both the Behaviour Editor and the Formation Editor
  appear, that the map stops scrolling, and that the two can be docked together.
- Check that the behaviour editor draws its paths, guides and handles and takes
  the mouse when editor mode opens, and that clicking in the world view does not
  hand the view to the formation editor.
- Click the Formation Editor's title bar and check the behaviour overlay goes,
  the formation markers appear, and the mouse now edits slots. Click back to the
  Behaviour Editor and check the overlay swaps back.
- Select a built-in (`line`, `column`, `v`, `grid`) and check it previews from the
  count and the spacing, has no slot list, and that Save is disabled.
- Select a database formation, then add slots by clicking in the world view, drag
  one, and right-click one to remove it. Check the panel list matches, and that
  the numbers there are unit space, not pixels.
- Change the count and check that slots beyond it are drawn smaller and fainter.
- Change the scale and rotation and check the preview follows, and that a slot
  added afterwards still lands under the mouse.
- Type a `Save as` name, press Save, and check `data/formation_database.json`
  holds the shape and that restarting the game loads it.

### [done] T-021 Fill the counted slots with the enemy's sprite

**Review:** reviewed 2026-09-20

**Depends on:** T-020

**Goal:**
A formation is judged by what it looks like when it is on the screen, so the
slots inside the count should be drawn as the enemy that will fill them rather
than as markers. Sprites, not spawned entities: nothing is added to the world, so
there is nothing to clean up when the count or the shape changes, and the preview
holds still while it is being edited.

The draw goes through the single-sprite function exposed in T-019, so a slot
looks the way the game draws that enemy.

**Requirements:**
- [x] The Formation Editor has its own combo over the enemy database, independent
      of the behaviour editor's.
- [x] Each slot within the count draws that enemy's sprite, through the function
      exposed in T-019.
- [x] Slots beyond the count keep their marker, so the whole shape is still
      visible while a smaller count is previewed.
- [x] No entities are created for the preview.
- [x] With no enemy chosen, the slots fall back to the markers from T-020 rather
      than drawing nothing.

**Out of scope:**
Animating the preview sprites, and anything to do with how the enemy flies. This
task draws the enemy standing in its slot.

**Open questions:**
- Q: Is the preview drawn markers, spawned enemies, or both? A: Markers for every
  slot, with the slots inside the count drawn as the selected enemy's sprite.
  Sprites rather than real spawns.
- Q: Where does the enemy come from? A: The formation editor's own enemy combo.

**Notes:**
Assumptions made: only the enemy's own sprite is drawn, not the sprites of its
sections, because a slot stands for one enemy; the preview sprite is drawn
upright, at no angle, since how the enemy flies is out of scope; an enemy that
has no sprite of its own falls back to the markers, as an unchosen one does.

The preview `ComponentSprite` is built for the frame rather than kept in a
static. A static would hold a texture past the graphics device at exit. Building
it is one animator allocation a frame, and it is built once for the whole shape,
not once a slot.

The selected-slot highlight is not drawn on a slot that has a sprite, because the
sprite replaces the marker there.

**Commits:**
- 07b921b feat(T-021): draw the enemy's sprite in the counted slots

**Manual verification:**
- Open the Formation Editor, choose an enemy, and check that the slots within the
  count draw that enemy's sprite and the slots beyond it stay markers.
- Lower and raise the count and check the sprites and markers swap over at the
  right slot.
- Change the enemy and check the sprites change with it.
- Clear the enemy choice, or pick an enemy that has no sprite, and check every
  slot falls back to a marker.
- Check that nothing is added to the world: no enemy flies, shoots or is hit
  while the preview is up, and leaving editor mode leaves nothing behind.

### [done] T-025 Let a spawn enter from any screen edge

**Review:** reviewed 2026-09-24

**Depends on:** none

**Goal:**
Enemies can only enter from the top of the screen. A spawn is placed at its Tiled
position when that position comes within `SpawnMargin` of the top of the screen,
and a `SpawnRelative` behaviour is anchored there. A `World` behaviour can start
elsewhere, but it throws away the formation offsets, so every member of a
formation follows the same path. The example wanted: a line of 5 enemies that
flies in from the bottom of the screen, hovers, fires and then moves off screen.

Add an `edge` property (`Top`, `Bottom`, `Left`, `Right`) to `EnemySpawn`. The
Tiled position of the object only governs *when* the spawn triggers. *Where* the
formation appears comes from a separate screen position on the spawn, snapped to
just outside the chosen edge. The edge also rotates both the formation offsets and
the behaviour, so every behaviour is authored once as "enter from the top" and
works from any edge.

**Requirements:**
- [x] `tiled_project/tincan.tiled-project` gains a hand-authored enum `edge` with
  the values `Top`, `Bottom`, `Left`, `Right`, in that order, and `EnemySpawn`
  gains the members `edge` (default `Top`), `use_position` (bool, default
  `false`), `position_x` and `position_y` (floats, default 0). This change is
  allowed despite the profile's Compatibility section, because it is additive:
  no existing type, member or value is changed or reordered. The `edge` enum is
  not added to `tiled_project/extensions/game_data_enums.js`, because it does not
  come from the game data.
- [x] The Tiled y position of the object still sets when the spawn triggers,
  whatever the edge, exactly as today.
- [x] With `use_position` set, `position_x`/`position_y` is a point in screen
  pixels. The formation appears just outside the chosen edge (by `SpawnMargin`),
  level with that point: `Top` and `Bottom` use `position_x`, `Left` and `Right`
  use `position_y`. The other coordinate is ignored.
- [x] With `use_position` unset, the point is the object's Tiled x and the middle
  of the screen height. A `Top` spawn with `use_position` unset behaves exactly
  as today, so the existing maps play unchanged.
- [x] The edge adds a rotation (`Top` 0, and a quarter or half turn for the others,
  so that "moving down the screen" becomes "moving in from that edge"). It is
  added to the Tiled object rotation for the formation offsets, and to
  `behaviour_rotation` (plus `behaviour_rotation_step` per member) for the
  behaviour.
- [x] A `World`-space behaviour's path is not rotated by the edge. The edge still
  places the formation and rotates its offsets, and `behaviour_rotation` still
  applies to it as today.
- [x] Enemies spawned at the new positions are not killed straight away by their
  bounds, including formation members whose rotated offset puts them further
  outside the screen.

**Out of scope:**
Editing the maps in `tiled_project/maps/`: no spawn is added or changed. Showing
the edge in the behaviour or formation editors. T-026's firing flag. An enemy with
no behaviour still drifts with the map scroll as today, so from a non-`Top` edge
it may never come on screen; that is accepted.

**Open questions:**
- Q: For `Left` and `Right`, what sets the position down the screen? A: The Tiled
  object's position only sets when the spawn triggers. Where it appears is a
  separate x,y screen point on the spawn (`position_x`, `position_y`), snapped to
  just outside the edge: Top/Bottom use x, Left/Right use y.
- Q: How do existing spawns, which have no x,y, keep working? A: A `use_position`
  bool, default false. Unset uses the Tiled x, so existing maps are unchanged.
- Q: How does the edge rotation combine with an existing `behaviour_rotation`
  and the spawn's own rotation? A: They are all added together.
- Q: What does the edge do to a `World`-space behaviour? A: It places the
  formation and rotates its offsets, but does not rotate the path.
- Q: May the session change `tincan.tiled-project`? A: Yes, additively only, as
  in the first requirement. Maps are not edited.

**Notes:**
- The edge rotations are `Top` 0, `Bottom` 180, `Left` -90 and `Right` 90
  degrees, clockwise on screen, so "down the screen" becomes "in from the edge".
- A `Top` spawn keeps the y it has today (its Tiled y less the camera top), with
  or without `use_position`. At the moment it triggers that is `SpawnMargin`
  above the screen, but a spawn that is already in view when the map starts
  still appears where it was placed, as today.
- `Bottom`, `Left` and `Right` place the front of the formation exactly
  `SpawnMargin` outside the edge, whenever the spawn triggers.
- Bounds: the edge side of each member's bounds moves out by its rotated offset
  along the edge normal, the same rule that already applied to the top edge. The
  other sides keep the screen plus `2 * SpawnMargin`.
- An `edge` value that is not one of the four logs an error and falls back to
  `Top`.
- `behaviour.space` decides the edge rotation. A spawn with no behaviour has the
  default `SpawnRelative`, but with no path there is nothing to rotate.

**Commits:**
- 1dcf1d2 feat(T-025): let a map spawn enter from any screen edge

**Manual verification:**
1. Open `tiled_project/tincan.tiled-project` in Tiled. The `edge` enum lists
   `Top`, `Bottom`, `Left`, `Right`, and an `EnemySpawn` object shows `edge`,
   `use_position`, `position_x` and `position_y` with their defaults. Existing
   maps open with no changes.
2. Play the existing maps. Every spawn enters from the top as before.
3. On a scratch copy of a map, add a spawn with `edge` `Bottom`, `count` 5,
   formation `line` and a spawn-relative behaviour such as `in_and_hold`. The
   five enemies appear in a line below the screen when the camera reaches the
   object's y, fly up into the screen and follow the behaviour turned upside
   down. None dies straight away.
4. Repeat with `Left` and `Right`. The line enters from that side, at the middle
   of the screen height, heading across the screen.
5. Set `use_position` and a `position_y` (for `Left`/`Right`) or `position_x`
   (for `Top`/`Bottom`). The formation enters level with that point.
6. Use a `column` formation from `Left`. The members behind the front start
   further off screen to the left and are not killed before they enter.
7. With a `World`-space behaviour and `edge` `Right`, the formation starts off
   the right edge, but each member follows the path as authored (not turned).

### [done] T-026 Fire enemy weapons only in chosen behaviour states

**Review:** reviewed 2026-09-24

**Depends on:** none

**Goal:**
Enemy weapons fire from the moment the enemy spawns until it dies, whatever its
behaviour is doing. It is active from `InitWeapon`, and nothing turns it off. So
an enemy cannot fly in quietly, hover and shoot, and then leave quietly. It also
fires while it is still off screen, which gives shots the player cannot see
coming.

Let each behaviour state say whether the enemy fires during it, with a `fire`
flag. The behaviour system turns the weapons on when a state that fires starts,
and off when one that does not starts. The existing `Weapon::m_active` switch is
the mechanism: a weapon switched off finishes a burst already under way, and its
cooldown keeps running, so it can fire as soon as it is switched back on.

**Requirements:**
- [x] `BehaviourStateData` has a `fire` flag, for every state type, read as an
  optional field that defaults to `true`, so existing behaviours fire as today.
  `to_json` writes it only when it is `false`.
- [x] When a state starts, the enemy's weapons are switched on or off to match its
  `fire` flag: the root entity's weapon and the weapons of all its sections.
- [x] After the last state ends, the weapons keep the last state's setting. An
  enemy with no behaviour fires as today.
- [x] The behaviour editor shows the flag as a checkbox on each state, and saving a
  behaviour keeps it.

**Out of scope:**
The player's weapons. A general rule against firing while off screen: authors set
`fire: false` on the entry state instead. Changing weapon timing, cooldowns or
bursts.

**Open questions:**
- Q: What is the default for a state that does not say: fire (as today, so
  existing behaviours are unchanged) or do not fire? A: Fire.
- Q: Do the weapons on enemy sections follow the root's behaviour state too?
  A: Yes.
- Q: What happens after the last state, or with no behaviour? A: The last state's
  setting stays; an enemy with no behaviour fires as today.
- Q: Should this task stop firing while off screen? A: No, `fire: false` on the
  entry state covers it.
- Q: Should the behaviour editor show and edit the flag? A: Yes.

**Notes:**
- The weapons are switched in `SystemBehaviour::Update` at the point where a
  state is entered, before its handler's `Init`. Map spawns, the behaviour
  system and the weapon system run in that order in one frame, so an entry state
  with `fire: false` stops the enemy firing before its weapon first updates.
- Sections are found through the root's `ComponentEntity::m_children`, which
  `SystemLifetime` keeps free of destroyed sections.
- The editor checkbox is labelled "Fire" and sits under the duration, for every
  state type.
- No behaviour in `data/behaviour_database.json` was changed, so every existing
  behaviour fires as before.

**Commits:**
- 0652d35 feat(T-026): fire enemy weapons only in behaviour states that allow it

**Manual verification:**
1. Play a map with armed enemies. They fire as before (no behaviour sets
   `fire: false` yet).
2. Open the behaviour editor (F2) and load a behaviour with several states,
   such as `in_and_hold`. Each state shows a "Fire" checkbox, ticked.
3. Untick "Fire" on the first state and save. In
   `data/behaviour_database.json` that state has `"fire": false`, and the other
   states have no `fire` key.
4. Play a map that spawns an armed enemy with that behaviour. It does not fire
   while it flies in, and starts firing when the next state starts.
5. Untick "Fire" on the last state as well. The enemy stops firing when that
   state starts and stays silent after the behaviour ends. A burst already
   under way when the state starts still finishes.
6. No enemy in `data/enemy_database.json` has a section with a weapon yet. To
   check sections, give a section of a test enemy a weapon. The section's
   weapon follows the same states as the root.
7. Reload the behaviour in the editor. The unticked boxes are still unticked.

### [done] T-029 Move the spawn edge into the behaviour

**Review:** reviewed 2026-09-24

**Depends on:** none

**Goal:**
Move the entry edge from `EnemySpawn` (added in T-025) into the behaviour, so
the behaviour itself designs how the enemy enters and behaves. The user is not
sure it is better, but it seems to make more sense as part of the behaviour.

With the edge on the spawn, a behaviour is authored once entering from the top
and rotated for other edges, so what plays is not what the behaviour editor
shows. With the edge on the behaviour, paths are authored as they play and
nothing is rotated. The spawn reads the edge from its behaviour to place the
formation just outside that edge, turn the formation offsets and extend the
bounds, all as T-025 does today. `use_position`, `position_x` and `position_y`
stay on the spawn, since where along the edge is a level design choice. The
cost is one behaviour per edge for the same move, with the spawn's
`behaviour_rotation` still available for variations.

The behaviour editor canvas is already authoring space: waypoints are drawn at
the screen coordinates they hold, and a spawn-relative path's first waypoint is
moved onto the spawn point at spawn. So a path drawn coming up from the bottom
already plays as drawn, and the preview needs no change.

**Requirements:**
- [x] `BehaviourData` has an `edge` field (`Top`, `Bottom`, `Left`, `Right`,
  in that order), read as an optional field that defaults to `Top`. `to_json`
  writes it only when it is not `Top`.
- [x] A map spawn takes its edge from its behaviour. A spawn with no behaviour,
  or an unknown one, enters from `Top`. Placement, the `use_position` point,
  the formation offset rotation and the bounds work as T-025 made them, driven
  by the behaviour's edge.
- [x] The edge no longer rotates the behaviour's path, for any behaviour
  space. `behaviour_rotation` and `behaviour_rotation_step` on the spawn still
  apply as today.
- [x] A `World`-space behaviour's edge places the formation and turns its
  offsets, as T-025 does today.
- [x] `tiled_project/tincan.tiled-project` no longer has the `edge` member on
  `EnemySpawn` or the `edge` enum type. Everything else in it is unchanged.
- [x] The behaviour editor shows and edits the edge as a combo on the
  behaviour, and saving a behaviour keeps it.
- [x] `data/behaviour_database.json` gains a Bottom version of `in_n_out`
  (for example `in_n_out_bottom`): `edge` `Bottom`, its paths mirrored to come
  up from the bottom of the screen and go back down, with the same states,
  durations and `fire` flags.
- [x] In `tiled_project/maps/stars.tmj`, the spawn that uses `edge` `Bottom`
  uses the new Bottom behaviour, and neither spawn has an `edge` property.
  Nothing else in the map changes.

**Out of scope:**
T-028's facing. Any change to the behaviour editor's preview, guides or spawn
button. `tiled_project/maps/desert.tmj`, which has no `edge` property. Other
behaviours in `data/behaviour_database.json`.

**Open questions:**
- Q: Is the edge removed from `EnemySpawn`, or kept as an override? A: Removed,
  with the `edge` enum type. The behaviour is the only source. This removal
  from the Tiled project is approved despite the profile's Compatibility
  section.
- Q: Are a behaviour's paths authored as they play from its edge (no rotation),
  or authored from the top and rotated by the behaviour's edge? A: As they
  play. The edge only places the formation and turns its offsets.
- Q: What does a spawn with no behaviour, or a `World`-space behaviour, do
  about the edge? A: No behaviour enters from `Top`. A `World`-space behaviour's
  edge places the formation and turns its offsets.
- Q: Should the behaviour editor show and edit the edge, and preview the path
  from it? A: It shows and edits it. The preview needs no change, because the
  canvas already shows paths as they play.
- Q: Is the edge per behaviour, or per state? A: Per behaviour.
- Q: Are the stars map test spawns updated as part of this task? A: Yes. The
  session adds a Bottom `in_n_out` behaviour, points the Bottom spawn at it and
  removes `edge` from both spawns.

**Notes:**
- The enum is `BehaviourEdge`, next to `BehaviourSpace` in
  `src/gamedata_behaviour.h`. The editor's combo sits under "Space".
- `in_n_out_bottom` mirrors `in_n_out` about the world height (1080 px), so its
  waypoints are y 1112 to 880 and back. A spawn-relative path's first waypoint
  is moved onto the spawn point, so only the shape matters at play time.
- `in_n_out_bottom` is not in the `behaviour` enum in
  `tiled_project/tincan.tiled-project`, because the requirement keeps
  everything but `edge` in that file unchanged. The enum stores strings, so
  the game reads the name, but Tiled does not offer it in the combo. Adding it
  is a follow-up (see Discovered).
- The launch check passed with the default 5 s and with 20 s, which covers
  both stars edge spawns (they trigger about 5.3 s in) and their full
  in/hold/out.

**Commits:**
- 075bc22 feat(T-029): move the spawn entry edge into the behaviour

**Manual verification:**
1. Start the game on the stars map. After about 5 s, the `danger_1_light`
   line comes in from the top at x 544, holds, and leaves back up.
2. At the same time, the `danger_1_dark` line comes up from the bottom at
   x 544, holds about 170 px above the bottom, and goes back down. Its
   formation is laid out below the front member, not above it.
3. Other spawns (for example the angled `danger_3` waves and asteroids) move
   as before.
4. Press F2. Load `in_n_out_bottom`: the Edge combo shows `Bottom`, and the
   paths are drawn near the bottom of the canvas. Load `in_n_out`: Edge shows
   `Top`.
5. Change a behaviour's Edge, save it under a new name, restart, and load it
   again: the Edge is kept. Check that the saved entry in
   `data/behaviour_database.json` has `"edge"` only when it is not `Top`, then
   remove the test entry.
6. Open `tiled_project/tincan.tiled-project` in Tiled. `EnemySpawn` has no
   `edge` member, and there is no `edge` enum. The stars map loads without
   property-type warnings.

### [done] T-031 Make "edit in view" a toggle in the behaviour editor

**Review:** reviewed 2026-09-25

**Depends on:** none

**Goal:**
In the behaviour editor, "edit in view" should be a toggle. Today, once it is
turned on, it cannot be turned off again without selecting another step.

It is an `ImGui::RadioButton` on `selectedState` in
`imgui_ext::InputBehaviourData` (`src/system_behaviour_editor.cpp`), and a
radio button can only be turned on.

**Requirements:**
- [x] "Edit in view" on a path step is a toggle. Turning it off on the step it
  is on for leaves no step edited in view.
- [x] Only one step is edited in view at a time. Turning it on for a step turns
  it off for the step that had it, as today.
- [x] Turning it off clears the selected and dragged waypoint, as loading a
  behaviour does. With no step edited in view, the canvas does not add, move
  or delete waypoints, and every path is drawn as a faint reference path.

**Out of scope:**
The collapsible step sections (T-032). Any other change to the editor canvas,
preview or guides.

**Notes:**
- The radio button is now an `ImGui::Checkbox` whose value is `i == selectedState`.
  Ticking it sets `selectedState` to that step, which takes it from any other step.
  Unticking it sets `selectedState` to -1.
- The selected waypoint is cleared wherever no path step is edited in view, next to
  the existing clear of the dragged waypoint. This also covers deleting the edited
  step, or changing its type away from Path.
- Assumption: switching "Edit in view" straight from one step to another keeps the
  selected waypoint index, as it did before. The requirement covers turning it off only.

**Commits:**
- 541abd4 feat(T-031): make edit in view a toggle in the behaviour editor

**Manual verification:**
1. Start the game, press F2, and load a behaviour that has two or more path steps.
2. Tick "Edit in view" on step 0. Its waypoints show handles in the game view.
3. Tick it on step 1. Step 0 unticks, and step 1's handles show.
4. Untick it on step 1. No step is ticked, no waypoint shows `>` in the panel, and
   every path is drawn faint with no handles.
5. With nothing ticked, left-click and right-click in the game view. No waypoint is
   added, moved or deleted.

### [done] T-032 Make behaviour steps collapsible sections

**Review:** reviewed 2026-09-25

**Depends on:** T-031

**Goal:**
The behaviour steps should be collapsible sections using the input label as a
heading, or "Step N" if empty.

The "input label" is the step's `Label` field (`BehaviourStateData::label`).
The steps are listed in `imgui_ext::InputBehaviourData`
(`src/system_behaviour_editor.cpp`), each as `N.` followed by its indented
fields.

**Requirements:**
- [x] Each behaviour step in the behaviour editor is a collapsible section.
- [x] The section heading is the step's label.
- [x] If the label is empty, the heading is "Step N", where N is the step's
  index counted from 0 (the first step is "Step 0").
- [x] Sections start open.
- [x] The step's controls, including the "Edit in view" toggle and "Delete",
  stay inside the section.
- [x] Editing a step's label changes its heading without collapsing or
  resetting the section, and two steps with the same label open and close
  on their own.

**Out of scope:**
Renaming "States:" and "Add state" in the editor, or "state" in code, to
"step". The waypoint and modifier lists inside a step. The canvas.

**Open questions:**
- Q: Is N counted from 1 or from 0? A: From 0, matching the waypoint numbering
  and the state index that log warnings print.
- Q: Do sections start open or collapsed, and where do the step's controls go?
  A: Open, with the controls inside the section.

**Notes:**
- Each step is an `ImGui::CollapsingHeader` with `ImGuiTreeNodeFlags_DefaultOpen`.
  Its label is `<label or "Step N">###step` inside the existing `PushID(i)`, so the
  heading text changes with the label while the ID stays the step's index.
- The "Edit in view" checkbox is no longer on the same line as the `N.` number. It
  is now the first control inside the section, indented with the other fields.
- Assumption: a section's open state belongs to its place in the list. When a step
  is deleted, the steps after it take on the open state of the step before them.
- Assumption: a label that contains `##` or `###` is shown as ImGui treats such a
  label. The text after the marker is hidden. Labels like that are not expected.

**Commits:**
- 5e9df33 feat(T-032): show behaviour steps as collapsible sections

**Manual verification:**
1. Start the game, press F2, and load a behaviour that has several steps.
2. Every step is an open section. A step with a label is headed by that label. A
   step with no label is headed "Step N", counted from 0.
3. Each section holds the step's "Edit in view" toggle (path steps only), its
   fields and "Delete". Collapse a section and check that all of them are hidden.
4. Type in a step's Label field. The heading follows the text, and the section
   stays open and keeps focus in the field.
5. Give two steps the same label. Collapse one, and the other stays open.
6. Clear a label. The heading goes back to "Step N".

### [done] T-033 Keep editor keypresses out of the game

**Review:** reviewed 2026-09-25

**Depends on:** none

**Goal:**
When typing in an editor field, keypresses should not go through to the game.

Today nothing filters key events for ImGui, in TinCan or in moth_toolkit.
`GameLayer::OnKeyEvent` acts on Quit (Escape), ToggleProfiler (F1) and
ToggleEditor (F2) on key-up. `GameWorld::OnKeyEvent` toggles Pause (P) and
passes every key to `SystemInput::OnKey`, which drives movement, fire, the
power weapon and affinity swap. So Escape in a text field quits the game, and
the arrow keys move the player.

**Requirements:**
- [x] While an ImGui text field is active (`io.WantTextInput`), a key press
  triggers no game binding: not Quit, ToggleProfiler, ToggleEditor, Pause or
  any `SystemInput` action.
- [x] While an editor window only has focus and no text field is active, keys
  reach the game as today.
- [x] A game key that is held when a text field becomes active is still
  released when the key comes up. It does not stay held.
- [x] The fix is in TinCan. moth_toolkit is not changed.

**Out of scope:**
Mouse input. Blocking keys whenever an ImGui window has focus
(`io.WantCaptureKeyboard`).

**Open questions:**
- Q: Does the game stop getting keys only while a text field is active, or
  whenever an ImGui window has focus? A: Only while a text field is active.
- Q: Which bindings are blocked while typing? A: All of them, including
  Escape, P, F1 and F2, so Escape cancels the edit rather than quitting.

**Notes:**
- The filter is at the top of `GameLayer::OnKeyEvent`, which runs before the event
  is dispatched to `GameWorld`. While `ImGui::GetIO().WantTextInput` is set, a key-down
  is marked handled, so neither the layer bindings, Pause nor `SystemInput` see it.
- A key-up always reaches `GameWorld`, so `SystemInput::OnKey` releases a key that
  was held before the text field became active. The layer bindings (Quit,
  ToggleProfiler, ToggleEditor act on key-up) are skipped for a key-up while typing.
- `GameLayer::m_keysPressedWhileTyping` remembers keys that went down while typing.
  Their key-up fires no layer binding even after the field has closed. Without
  this, the Escape that cancels an edit would quit on release, because ImGui closes
  the field on the key-down.
- Marking the key-down handled cannot starve ImGui. The GLFW backend is set up with
  `install_callbacks = true`, so ImGui reads keys from GLFW, not from layer events,
  and GameLayer is the only layer on the stack.
- `WantTextInput` is the value from the last ImGui frame. A key pressed on the same
  frame that a field is clicked can still reach the game.

**Commits:**
- c035d4e fix(T-033): keep keypresses in editor text fields out of the game

**Manual verification:**
1. Start the game and press F2 to open the editors.
2. Click into a text field, for example "Save as" in the Behaviour Editor.
3. Press the arrow keys, fire, P, F1 and F2. Text changes in the field only. The
   player does not move or fire, the game does not pause, and the profiler and
   editors do not toggle.
4. Press Escape. The edit ends and the game does not quit. Press Escape again,
   with no field active, and the game quits.
5. Restart. Hold an arrow key so the player moves, click into a text field with the
   mouse while holding it, then let go. The player stops moving.
6. Click on an editor window but not in a text field (for example on a combo or
   empty space). The arrow keys, P, F1 and F2 work as before.
