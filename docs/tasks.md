# Tasks

The task list for TinCan. The process, the status values and the field
definitions are in the shared workflow, `/home/mcotton/Development/moth/.claude/tasks/workflow.md`.

- `/task-new <description>` adds a task.
- `/task-planning [IDs]` reviews tasks.
- `/task-session` implements reviewed tasks.

Session reports are in [sessions/](sessions/). `/task-planning` moves finished tasks
to [tasks-done.md](tasks-done.md).

## Tasks

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

## Discovered`, and left unfixed because it was outside every task in that
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

### [in-progress] T-017 Document the path entry rule

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
- [ ] `docs/behaviour-paths-and-editor.md` describes the `entry` field: its three
      values, that it runs before the state's duration starts counting, and that
      it covers the spawn into the first state.
- [ ] The document says how `entry` interacts with T-011's anchoring, once T-011
      is done: a spawn-relative behaviour's first path state begins where the
      enemy already is, so its entry rule has nothing to do.
- [ ] Prose is wrapped at about 85 columns, as the workspace `CLAUDE.md` asks.
- [ ] No code changes.

**Out of scope:**
Changing how `entry` works. This task only writes down what it already does.

**Notes:**

**Commits:**

**Manual verification:**

## Discovered
