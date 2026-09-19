# Tasks

The task list for TinCan. The process, the status values and the field
definitions are in the shared workflow, `/home/mcotton/Development/moth/.claude/tasks/workflow.md`.

- `/task-new <description>` adds a task.
- `/task-planning [IDs]` reviews tasks.
- `/task-session` implements reviewed tasks.

Session reports are in [sessions/](sessions/). `/task-planning` moves finished tasks
to [tasks-done.md](tasks-done.md).

## Tasks

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

- `SystemPickup::Update` logs `[error] Unable to locate player` and then
  dereferences a null `playerPosition` in the pickup loop that follows
  (`src/system_pickup.cpp:132`-`152`). Reachable once the player entity is gone.
  Found by the T-008 launch check.
