# Waypoint paths, movement modifiers and the behaviour editor

Plan for replacing TinCan's movement behaviour types with a single waypoint path
plus stackable modifiers, and for rebuilding the behaviour editor around direct
mouse editing of those paths.

Target branch: `main`.

## Goal

Two things, and they depend on each other.

**A behaviour becomes a list of states, where a movement state is geometry.** A
path state is a list of waypoints, an interpolation between them, and a speed.
Modifiers layer on top: a wave applied to a straight two-point path gives wavy
movement, and the same wave applied to a curve gives a wavy curve. One concept
replaces `Line`, `Wave` and `Spline`.

**The editor becomes the place paths are authored.** Waypoints are placed,
dragged and deleted with the mouse in the game view at game scale, against a
chosen enemy from the database, and the result is saved back to
`data/behaviour_database.json`.

Existing behaviour data is test data and may be discarded. Nothing needs
migrating.

## Where things stand

The model:

- `BehaviourType` (`src/gamedata_behaviour.h:6`) is `Line`, `Target`, `Hold`,
  `Wave` and `Spline`.
- `BehaviourStateData` (`src/gamedata_behaviour.h:23`) is the union of every
  type's fields: `velocity`, `target`, `point`, `points`, `speed`, `frequency`,
  `amplitude`, plus `label` and `duration`. `from_json` picks which are required
  per type.
- `BehaviourData` (`src/gamedata_behaviour.h:36`) is a flat `states` vector.
  `ComponentBehaviour` (`src/system_behaviour.h:25`) walks it by index, advancing
  on `duration` alone.
- Each type has a handler (`src/system_behaviour.cpp:123`–`269`) with
  `Init`/`Update`/`Deinit`, selected through the `BehaviourHandlers` map
  (`src/system_behaviour.cpp:271`).
- `data/behaviour_database.json` holds three behaviours: `basic`, `chase_player`
  and `in_and_hold`. None uses `Spline` or `Wave`.

Three defects worth naming, because the plan fixes them by construction:

- **Empty point lists crash.** `SampleCatmullRomPath`
  (`src/utils_paths.h:14`) indexes `points.back()` and `points[0]` with no size
  check. Spawning an editor-built spline reaches it through
  `BuildArcLengthTable` (`src/utils_paths.h:38`) and takes the application down.
- **The editor and the runtime disagree about the same points.** `from_json`
  duplicates the first and last point so Catmull-Rom has its control points
  (`src/gamedata_behaviour.h:60`). The editor never runs `from_json`, so the
  curve it draws is not the curve the game flies. Below four points the sampler
  silently returns `points[0]` forever and the enemy sits still.
- **Coordinate space is implicit and inconsistent.** Spline positions are
  relative to the spawn point, via `m_offset` in `ComponentSplineCache`
  (`src/system_behaviour.h:20`). `Target`'s `point` is absolute. Same-looking
  data, different meaning, nothing says so.

The editor (`src/system_behaviour_editor.cpp`, toggled with F2):

- Edits function-local `static` `EnemyData` and `BehaviourData` in `Draw`
  (`:108`). Nothing loads, nothing saves, everything is lost on exit.
- `InputEnemyData` (`:8`) asks for sprite, radius and health rather than naming
  an enemy from the database.
- Waypoints are typed as float pairs (`:28`), one `InputFloat2` per point.
- `DrawSpline` (`src/system_debug_spline.h:16`) draws the curve into the world
  surface, so a preview already reaches the screen in world space.
- `GameLayer::Draw` (`src/game_layer.cpp:59`–`67`) computes the window-to-world
  scaling and letterbox offset every frame and then discards it. That is exactly
  the transform mouse picking needs, inverted.

## The model

### A path state

A movement state carries:

- **`waypoints`** — the positions, in order.
- **`interpolation`** — `Linear` or `CatmullRom` to begin with. Room for `Step`
  and other curves later without touching anything else.
- **`space`** — `SpawnRelative` or `World`. Stated, not inferred from the type.
  Most authored paths are spawn-relative, so a formation can reuse one shape.
- **`speed`** — travel rate along the path, in pixels per second.
- **`duration`** — as now, with `0` meaning "until the path completes".

`Line` becomes two waypoints with `Linear`. A spline becomes waypoints with
`CatmullRom`. Both sample through one arc-length table, so speed means the same
thing whatever the interpolation, and the editor cannot draw a curve the game
does not fly.

Catmull-Rom's endpoint padding moves into the sampler, where it applies to
authored and loaded data alike. The sampler handles zero, one, two and three
waypoints as defined cases rather than crashing or silently freezing.

### Modifiers

A movement state carries a list of modifiers, applied in the order given to the
position the path produced. The starting set:

- **`Wave`** — oscillation perpendicular to the direction of travel, with
  amplitude and frequency. Replaces the `Wave` type outright.
- **`Phase`** — an offset into a modifier's cycle, so formation members on one
  shared behaviour do not move in lockstep.
- **`Ease`** — reshapes speed along the path, for accelerating in and slowing
  into a hold, without bending the geometry.

Each modifier is a small fixed set of numbers. Modifiers do not read the world,
do not branch and cannot be nested.

### Where the generalisation stops

Points and modifiers describe a **shape**. They cannot describe a **decision**.

`Target` and `Hold` stay as they are, and future reactive states join them
rather than being folded in. Chasing the player is not authored geometry: the
destination moves, so there is no path to bake and no arc-length table to
precompute. That is the honest technical boundary, and it is a good place to
stop — a system where waypoints resolve to live entities and modifiers take
expressions is a scripting language with a worse interface than text.

The rule: **if it can be baked at state entry, it belongs in the path; if it has
to be re-evaluated every frame against the world, it is a separate state type.**

What this leaves genuinely missing is conditional transitions — states advance
on `duration`, so nothing reacts to health, distance, or a formation leader
dying. That is real, and it is deliberately not in this plan. It is the next
question after this one, and it is easier to answer once movement is one concept
instead of five.

## Schema changes

`data/behaviour_database.json` entries keep their shape: a named behaviour with
a `states` list, each state having a `type`, an optional `label` and a
`duration`. What changes is the movement state's fields — `velocity`,
`frequency` and `amplitude` leave `BehaviourStateData`, `points` becomes
`waypoints` beside `interpolation` and `space`, and a `modifiers` list appears.

`BehaviourStateData` stays one struct holding the union of every type's fields,
matching how the file works today. The alternative — a variant per type — is
tidier and a bigger change to `from_json`, the handlers and the editor all at
once. It is listed under open decisions rather than assumed.

## The editor

### Choosing what to spawn

Two combos replace the typed enemy fields: one over the enemy database keys, one
over the behaviour database keys, loading a named behaviour as the starting
point. Both follow the sprite combo already in `InputEnemyData`. Editing a
behaviour that exists, rather than always starting blank, is most of what makes
the editor useful.

### Editing waypoints with the mouse

Waypoints are drawn as handles into the world surface, next to the path preview
`DrawSpline` already produces. Click on empty space to append, drag a handle to
move, right-click a handle to delete, with the path redrawing live.

This needs a window-to-world transform, which `GameLayer::Draw` already computes
and throws away. Lifting it into something both the draw and the editor can ask
for is the enabling change.

Editing in the game view rather than a separate canvas is deliberate: the
judgement being made is whether a movement feels right at the speed and scale
the player sees, and that is not a judgement a diagram supports.

The numeric fields stay, as a way to nudge a value precisely and to see what is
selected.

### Saving

The editor writes the edited behaviour back to `data/behaviour_database.json`
under a name. The gamedata structs are load-only today, so the movement state
needs a serialiser to match its loader.

Until that exists the editor cannot keep anything, which caps how much the
earlier steps are worth. It is late in the order below because the model should
settle first, but it is not optional.

### Previewing

A ghost scrubbed along the path, with direction and time markers, so a shape can
be read without spawning and watching. Cheapest way to make iteration fast, and
it depends on nothing else here.

## Suggested order of work

1. **Guard the samplers.** Defined behaviour for zero to three waypoints,
   endpoint padding moved out of `from_json` and into the sampler, Spawn
   disabled while a state is incomplete. Stops the crash before anything is
   built on top.
2. **Introduce the path state.** `waypoints`, `interpolation`, `space` and one
   arc-length sampler for both interpolations. `Line`, `Wave` and `Spline` and
   their handlers go. `data/behaviour_database.json` is rewritten by hand — it
   is three entries.
3. **Add modifiers.** `Wave` first, since it has to exist for step 2 not to lose
   a feature. `Phase` and `Ease` after.
4. **Enemy and behaviour combos in the editor.** Small, and immediately useful.
5. **Mouse editing.** Expose the window-to-world transform, draw handles, then
   pick, drag and delete.
6. **Saving.** Serialiser for the movement state, write back by name.
7. **Ghost preview.** Scrub along the path.

Steps 1 and 2 are the ones that must land together in spirit: after step 1 the
old types still exist, and step 2 is what makes the model worth having.

## Open decisions

- **One struct or a variant per state type.** `BehaviourStateData` is currently
  the union of every type's fields. A variant is tidier and touches the loader,
  the handlers and the editor at once. Deciding before step 2 is cheaper than
  after.
- **Whether modifier order is authored or fixed.** An explicit ordered list is
  more expressive; a fixed application order is one less thing to get wrong in
  the editor. Fixed order is the smaller starting point.
- **Whether `space` is per state or per behaviour.** Per state is more flexible.
  Per behaviour is likely what every real case wants and is simpler to show in
  the editor.

## Out of scope

**A general gamedata editor.** The enemy and behaviour combos in step 4 are a
hand-written slice of one: a panel that understands `DataRef`, offers a selector
for any referenced type, and builds its fields from the data structures. It
would replace per-type editor code across the whole database and is worth doing
on its own terms — but it is a separate, larger piece of work, and this plan
should not wait on it or quietly grow into it.

**Conditional transitions between states.** Discussed above under where the
generalisation stops. The next question, not this one.

## Verification

- The three behaviours in `data/behaviour_database.json` still read and run
  after the rewrite, with `chase_player` and `in_and_hold` unchanged in
  behaviour and `basic` expressed as a two-waypoint linear path.
- A path with zero, one, two and three waypoints each spawn without crashing.
- A path authored in the editor and the same path loaded from JSON produce the
  same movement.
- A wave modifier over a two-waypoint linear path matches what the old `Wave`
  type produced.
- Waypoints can be added, dragged and deleted with the mouse, and the drawn path
  matches the path flown.
- A behaviour saved from the editor reloads on the next run.
