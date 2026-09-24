# Tasks

The task list for TinCan. The process, the status values and the field
definitions are in the shared workflow, `/home/mcotton/Development/moth/.claude/tasks/workflow.md`.

- `/task-new <description>` adds a task.
- `/task-planning [IDs]` reviews tasks.
- `/task-session` implements reviewed tasks.

Session reports are in [sessions/](sessions/). `/task-planning` moves finished tasks
to [tasks-done.md](tasks-done.md).

## Tasks

### [todo] T-022 Give the player multiple lives

**Review:** unreviewed

**Goal:**
Add multiple lives for the player.

**Requirements:**
- [ ] The player has more than one life.

**Open questions:**
- Q: How many lives does the player start with, and is that a data value?
- Q: What happens when a life is lost: respawn where, after what delay, with
  what invulnerability if any?
- Q: What is kept and what is lost on death (weapons, power-ups, score)?
- Q: What happens when the last life is lost, and how does the game show the
  lives remaining?
- Q: Can lives be gained during play?

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-023 Pause the map scroll

**Review:** unreviewed

**Goal:**
Add map scroll pause support. Some areas might want to pause for a time or for
some kind of trigger, for example boss fights.

**Requirements:**
- [ ] The map scroll can be paused for a time.
- [ ] The map scroll can be paused until a trigger releases it.

**Open questions:**
- Q: How is a pause authored in the map data: a point on the map, a spawn, or
  something else?
- Q: What kinds of trigger release a pause (all spawned enemies dead, a named
  enemy dead, a timer, a script)?
- Q: Does anything else stop while the scroll is paused, for example the
  background layers, the parallax or the spawning?
- Q: Can a map hold several pauses, and can a pause be skipped?

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-024 Add intro and exit animations

**Review:** unreviewed

**Goal:**
Add intro/exit animation support. The player flying in and out, or something
like that.

**Requirements:**
- [ ] A map can play an intro animation, such as the player flying in.
- [ ] A map can play an exit animation, such as the player flying out.

**Open questions:**
- Q: Where does an intro or exit animation start and end: the map, a level, the
  whole game, or all of these?
- Q: Is the animation authored in data, or is it code?
- Q: What does the player control during the animation, and what happens to the
  scroll, the spawning and the enemies while it plays?
- Q: Does it also cover death and respawn (see T-022), or only entering and
  leaving a map?

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-027 Spawn enemies when an enemy is killed

**Review:** unreviewed

**Goal:**
Split a large asteroid into several smaller ones when it is killed, handled
generically on the data side rather than as an asteroid feature. Asteroids today
are plain enemies with no weapon.

The approach discussed: `EnemyData` gets a `death_spawns` list. Each entry
describes a spawn with the same fields as a Tiled `EnemySpawn` (enemy, behaviour,
count, formation, spacing, scale, speed, `behaviour_rotation`,
`behaviour_rotation_step`, drop). `behaviour_rotation_step` fans the pieces out,
so one short "drift out" behaviour gives pieces heading in different directions.
Large to medium to small is a chain of entries, and the same field covers carriers
that release fighters, mines that burst, and similar.

The spawn logic in `SpawnMapEnemy` (`src/system_map.cpp`) would be split into a
spawn description struct and a function that spawns from it, shared by map spawns
and death spawns, so pieces get bounds, rotated behaviours and phase index the same
way map spawns do.

The spawns would trigger in `SystemLifetime`, which already tells killed entities
from escaped ones, and not in `SystemDamage::ApplyDamage`, which runs inside view
loops over the pools a new enemy adds to.

**Requirements:**
- [ ] An enemy's data can list spawns that happen when it is killed.
- [ ] A death spawn uses the same fields as a map spawn.
- [ ] Death spawns happen only when the enemy is killed, not when it escapes.
- [ ] A large asteroid splits into smaller asteroids, and those can split again.

**Open questions:**
- Q: If the parent is in a group with a drop, do the pieces join that group, so
  the drop waits until the last piece is killed?
- Q: Does an enemy that dies by ramming the player (contact `self_damage`) spawn
  its pieces too?
- Q: Does `EnemySection` get `death_spawns` as well?
- Q: Do pieces carry on with any of the parent's movement, or only follow their
  own behaviour?

**Notes:**

**Commits:**

**Manual verification:**

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

### [todo] T-028 Add a facing property to EnemySpawn

**Review:** unreviewed

**Goal:**
Add a facing property to `EnemySpawn` so a spawned enemy can be rotated, for
example turned 180 degrees when it comes in from the bottom (T-025's `edge`).

Context from the conversation: spawns are Tiled point objects, and Tiled cannot
rotate a point, so the object's own rotation is always 0 and cannot be used for
this. An enemy's sprite is drawn at `ComponentEntity::m_angle`, which starts at 0
and is changed only by targeting, so today an enemy spawned from the bottom still
faces down. Barrels fire along `m_angle` too, so the facing also sets the
direction the enemy fires in. Sections are placed by an unrotated offset and
drawn at angle 0.

T-029 proposes moving the edge from the spawn to the behaviour. If that goes
ahead, the default facing may come from the behaviour's edge, or from each
behaviour state, rather than from the spawn. Plan the two tasks together.

**Requirements:**
- [ ] `EnemySpawn` has a facing property that rotates the spawned enemy.

**Open questions:**
- Q: What units and values does the property take: degrees, or a set of named
  directions?
- Q: Should the facing default from the spawn's `edge`, with the property as an
  override, or does it apply only when set?
- Q: Do the enemy's sections rotate with it, both their offsets and their
  sprites?
- Q: Should the facing also turn the direction the weapons fire, which follows
  from `m_angle` today?
- Q: Does the facing stay fixed, or follow the direction of travel along the
  behaviour?
- Q: If T-029 moves the edge to the behaviour, does the facing belong on the
  behaviour (or its states) instead of the spawn, and is a spawn property still
  needed?

**Notes:**

**Commits:**

**Manual verification:**

## Discovered

- `SystemMap::Update` (`src/system_map.cpp`, the `m_cameraTop` line) has a
  `readability-math-missing-parentheses` clang-tidy finding that was there
  before the 2026-09-24 session.
- The `behaviour` enum in `tiled_project/tincan.tiled-project` does not list
  `in_n_out_bottom` (added by T-029), so Tiled does not offer it in the
  `EnemySpawn` behaviour combo. The enum is kept by hand.
