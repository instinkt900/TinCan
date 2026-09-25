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

### [done] T-034 Rename the spawn position properties to spawn_x and spawn_y

**Review:** reviewed 2026-09-25

**Depends on:** none

**Goal:**
Change the `position_x` and `position_y` properties in Tiled objects to
`spawn_x` and `spawn_y`. This better explains what they do. When the `use_spawn`
toggle is on, the enemy or formation spawns at those screen coordinates.

Context: `EnemySpawn` has `use_position`, `position_x` and `position_y` today
(see T-029), read in `SpawnMapEnemy` (`src/system_map.cpp`). The tilemap loader
fills in class defaults for properties Tiled did not write, so the game cannot
tell an unset `spawn_x` from one set to its default. A toggle is needed.

No map in `tiled_project/maps/` sets `use_position`, `position_x` or
`position_y`, so no map data needs migrating. This task is allowed to change the
`EnemySpawn` class in `tiled_project/tincan.tiled-project`, which the task
profile otherwise lists as must-still-load.

**Requirements:**
- [x] The `EnemySpawn` class in `tiled_project/tincan.tiled-project` has
  `use_spawn`, `spawn_x` and `spawn_y` in place of `use_position`, `position_x`
  and `position_y`, with the same types and defaults.
- [x] The game reads `use_spawn`, `spawn_x` and `spawn_y`. When `use_spawn` is
  on, the enemy or formation spawns at those screen coordinates, as
  `use_position` does today.
- [x] Both maps still load and play as before.

**Out of scope:**
- Loading the old names. Nothing uses them.

**Open questions:**
- Q: Can the game tell that `spawn_x` and `spawn_y` are absent, or is a
  `use_spawn` toggle needed? If it is needed, does it replace `use_position`?
  A: A toggle is needed, because the loader fills in class defaults.
  `use_spawn` replaces `use_position`.
- Q: Are the existing maps migrated to the new names, and must the old names
  still load? A: No map uses them, so there is nothing to migrate, and the old
  names do not need to load.

**Notes:**
- The class members stay in Tiled's alphabetical order, so `spawn_x` and
  `spawn_y` sit between `spacing` and `speed`.
- No map sets the old names (checked with grep), so nothing was migrated.

**Commits:**
- 7df22bb refactor(T-034): rename spawn position properties to use_spawn,
  spawn_x and spawn_y

**Manual verification:**
1. Open `tiled_project/tincan.tiled-project` in Tiled and check that an
   `EnemySpawn` object shows `use_spawn`, `spawn_x` and `spawn_y`, with no
   `use_position`, `position_x` or `position_y`.
2. On a spawn, set `use_spawn` on and give `spawn_x`/`spawn_y` a screen point.
   Play the map and check that the enemy appears at that point, level with it on
   its entry edge, as `use_position` did before.
3. Play both maps without changes and check they play as before.

### [todo] T-035 Replace the behaviour edge with a formation rotation

**Review:** unreviewed

**Goal:**
Change the behaviour's `edge` enum into a number that rotates the formation by
a given amount.

Context from the conversation: today the edge does three things in
`SpawnMapEnemy` (`src/system_map.cpp`). It places the front of the formation
just outside a screen side. It turns the formation offsets by 0, 180, -90 or 90
degrees (`GetEdgeRotation`). And it pushes each member's kill bounds out on the
entry side. It does not turn the path. An angle covers the offset rotation
directly, but it does not say which screen side to place the formation at or
which side to push the bounds out on. The bounds could be pushed out by the
offset length on every side.

**Requirements:**
- [ ] The behaviour's `edge` enum is replaced by a number that rotates the
  formation by that amount.

**Out of scope:**
A rotation per behaviour step, interpolated between steps, so a formation can
spawn in one orientation and turn as it moves to the next step. This was
discussed and deliberately left for later.

**Open questions:**
- Q: Where is the formation placed, and which side do the bounds reach out on,
  once there is no edge?
- Q: Is the number in degrees, and what is the field called?
- Q: What happens to existing behaviours that have `edge` set, for example
  `in_n_out_bottom`?

**Notes:**

**Commits:**

**Manual verification:**

### [done] T-036 Add lifetime and screen-exit rules for escaping enemies

**Review:** reviewed 2026-09-25

**Depends on:** none

**Goal:**
In relation to T-035, add rules for how enemies get culled or "escaped". When
an enemy spawns, it gets a lifetime counter, defined in data, per enemy. If the
enemy survives longer than that time, it is "escaped" and removed from the
game. Once an enemy enters the visible screen area, it becomes "active". Once
active, if it leaves the bounds of the visible area plus a small hardcoded
border, it is also "escaped" and removed. The timer continues while the enemy
is active and is the hard limit on an enemy's lifetime.

Context from the conversation: T-035 removes the behaviour's edge, which today
decides which side each member's kill bounds are pushed out on so that members
still waiting off screen are not removed. These rules would replace the need
for that.

Context from planning: every enemy is created by `SystemEnemySpawner::SpawnEnemy`
(map spawns and the behaviour editor's Spawn button), so the rules can be set up
there. `ComponentLifetime` (`src/system_lifetime.h`) already exists and nothing
uses it yet. An escape already reaches groups through `EscapedTag`, so an
escaped member cancels its group's drop as today. Enemies spawned from the
behaviour editor have no bounds today and live until killed; under these rules
they expire too.

At `scroll_speed` 60 on the 1080 px screen, an enemy with no behaviour takes
about 19 seconds to cross the screen, so the 15-second default removes it before
it reaches the bottom. That is accepted: enemies used that way set a longer
lifetime in their data.

**Requirements:**
- [x] Each enemy's data can set a `lifetime` in milliseconds. It defaults to
  15000 when not set. `data/enemy_database.json` still loads unchanged.
- [x] An enemy that lives longer than its lifetime is escaped and removed.
- [x] An enemy becomes active when its position is inside the visible screen.
- [x] An active enemy whose position leaves the visible screen grown by 64 px
  on every side is escaped and removed.
- [x] An enemy that has never been active is removed only by its lifetime.
- [x] The lifetime keeps counting while the enemy is active.
- [x] An escaped enemy's sections are removed with it.
- [x] These rules replace all existing enemy culling. `SpawnMapEnemy` no longer
  gives enemies kill bounds.
- [x] All enemies follow these rules, including those spawned with no behaviour
  and those spawned from the behaviour editor.

**Out of scope:**
- Projectile and pickup culling.
- Changing any enemy's lifetime in `data/enemy_database.json`.

**Open questions:**
- Q: What units is the lifetime in, and what happens when an enemy's data does
  not set it? A: Milliseconds, with a default of 15 seconds.
- Q: Do these rules replace the current kill bounds (`ComponentBounds` with
  `BoundsBehaviour::Kill`) for enemies completely? A: Yes. They replace any
  existing enemy culling.
- Q: Does an enemy spawned with no behaviour, which moves with the map, follow
  the same rules? A: Yes. All enemies follow the same cull rules.
- Q: The 15-second default removes map-drifting enemies before they cross the
  screen. What should happen? A: Keep the 15-second default. Enemies used that
  way set a longer lifetime per enemy.
- Q: What counts as entering the screen, and how big is the border? A: The
  enemy's position (its centre) is inside the screen. The border is 64 px.

**Notes:**
- `SpawnEnemy` gives every enemy root a `ComponentLifetime` (from
  `EnemyData::lifetime`) and a new `ComponentScreenExit` (`m_active`).
  `SystemWorldBounds` activates it and removes an active enemy that leaves the
  screen grown by 64 px, because that system already has the world size.
- An expired lifetime now sets `DeadTag` and `EscapedTag` instead of removing
  the entity directly, so `UpdateDead` also removes its sections. Before this,
  an expired entity's children were left for `UpdateParenting` to catch a frame
  later. `ComponentLifetime` had no users, so nothing else changes.
- The lifetime pass skips entities already marked dead, so an enemy killed in
  the frame its lifetime runs out still counts as killed for its group's drop.
- `ComponentBounds` is untouched and still culls projectiles and limits the
  player.

**Commits:**
- 94a22c0 feat(T-036): cull enemies by lifetime and by leaving the screen once
  active

**Manual verification:**
1. Play a map. Enemies that fly across and off the screen disappear shortly
   after leaving it (within 64 px of the edge), from any side.
2. A formation that waits or flies in from off screen is not removed before its
   members come on screen, including members at the back of a long formation.
3. Kill a whole formation with a drop: the drop still appears. Let one member
   fly off screen: no drop.
4. Add `"lifetime": 3000` to an enemy in `data/enemy_database.json`
   temporarily, spawn it, and check it vanishes after about three seconds even
   while on screen. Revert the change.
5. Spawn an enemy with sections (a multi-part enemy) and let it escape or time
   out: its sections go with it.
6. In the behaviour editor (F2), spawn an enemy and leave it: it is removed
   after 15 seconds, or when it leaves the screen after being on it.
7. An enemy spawned with no behaviour drifts down with the map and is removed
   after 15 seconds, before it reaches the bottom. This is accepted.

### [todo] T-038 Loop behaviours from a chosen state

**Review:** reviewed 2026-09-25

**Depends on:** none

**Goal:**
Let a behaviour repeat, so repeating movement does not have to be authored for
the whole length of the screen. A zigzag down the screen should be one short
left-right segment set to loop, and other repeating patterns should work the
same way, such as travelling straight for a time, chasing the player for a time,
then travelling straight again.

A behaviour gets an optional state to loop from. After its last state finishes,
it goes back to that state and repeats, forever. Looping from the first state is
a plain loop; looping from a later one allows an intro (fly in from the edge)
before the repeating part.

Spawn-relative states are all measured from the spawn origin, so a naive loop
would replay the same segment in the same place. Each time the behaviour loops,
the origin moves by how far the entity travelled over the last pass (its position
when it entered the loop's first state compared with now). A segment such as
`(0,0) → (60,40) → (0,80)` then keeps travelling down the screen, and a
straight/chase/straight cycle carries on from wherever the chase left the enemy
instead of snapping back towards the spawn point. World-space behaviours do not
move their origin, so a looping world path is a fixed patrol.

Context from planning: the state sequencing is in `SystemBehaviour::Update`
(`src/system_behaviour.cpp`), and the origin is `ComponentBehaviour::m_origin`
(`src/system_behaviour.h`). A behaviour that loops forever and never leaves the
screen is removed by the enemy lifetime from T-036, when that is done.
`data/behaviour_database.json` is disposable test data, so its schema may
change.

**Requirements:**
- [ ] A behaviour can name a state to loop from. With none, it behaves as today,
  and behaviours without one still load.
- [ ] After the last state, a looping behaviour continues from that state,
  indefinitely.
- [ ] A spawn-relative behaviour's origin moves by the entity's displacement over
  each pass, so each pass starts where the last one ended.
- [ ] A world-space behaviour's origin does not move when it loops.
- [ ] A loop whose pass uses no time does not spin forever within one update.
- [ ] The behaviour editor can set the state to loop from, and can clear it.
- [ ] Deleting a state in the editor keeps the loop pointing at the same state,
  or clears it when that state is the one deleted.
- [ ] The editor preview shows three repeats of the loop, shifted as they would
  be in game.
- [ ] The behaviour is saved and loaded with its loop state.

**Out of scope:**
- A repeat count (loop N times, then continue). Deferred until a behaviour needs
  it.
- A triangle waveform for the `Wave` modifier.

**Open questions:**
- Q: A triangle waveform option for the `Wave` modifier would give a sharp
  zigzag without looping. Is that part of this task, or a separate one?
  A: Neither. Looping covers the zigzag, so it is dropped.

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-027 Spawn enemies when an enemy is killed

**Review:** reviewed 2026-09-25

**Depends on:** T-036

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
and death spawns, so pieces get rotated behaviours and phase index the same way
map spawns do.

The spawns would trigger in `SystemLifetime`, which already tells killed entities
from escaped ones, and not in `SystemDamage::ApplyDamage`, which runs inside view
loops over the pools a new enemy adds to.

Context from planning: this depends on T-036 because T-036 moves enemy culling
into `SpawnEnemy`, so pieces get the lifetime and screen-exit rules without this
task having to copy the edge-based kill bounds that T-036 removes. A death
spawn is placed at the parent's position; the map spawn's screen edge and spawn
point do not apply to it. There are no smaller asteroid sprites and no enemy
scale, so the asteroid chain is shown with the existing asteroid entries.

**Requirements:**
- [ ] An enemy's data can list `death_spawns`. Each entry has the same fields as
  a map spawn, apart from where it spawns. `data/enemy_database.json` without
  them still loads.
- [ ] Map spawns and death spawns share one spawn path, so pieces get their
  behaviour copy, override, rotation step and phase index the same way map
  spawn members do.
- [ ] Death spawns are placed around the parent's position when it was killed.
- [ ] Death spawns happen when the enemy is killed by any means, including
  ramming the player (contact `self_damage`), and not when it escapes.
- [ ] If the parent was in a group, its pieces join that group, so the group's
  drop waits until every piece is gone, and a piece escaping cancels it as any
  member escaping does. A death spawn entry with its own drop also puts its
  pieces in a group of their own for that drop, as a map spawn does.
- [ ] Pieces with no behaviour move at the parent's velocity when it died, taken
  from its last step (position minus last position), so this works whatever the
  parent was doing. Pieces with a behaviour follow only their behaviour.
- [ ] `data/enemy_database.json` has a chain using the existing asteroids, for
  example `asteroid_1` into `asteroid_2` pieces into `asteroid_3` pieces,
  and a map spawn uses it.

**Out of scope:**
- `death_spawns` on `EnemySection`.
- Smaller asteroid art, or a scale on `EnemyData`.
- Adding inherited velocity to pieces that have a behaviour.

**Open questions:**
- Q: If the parent is in a group with a drop, do the pieces join that group, so
  the drop waits until the last piece is killed? A: Yes, pieces join the
  parent's group.
- Q: Does an enemy that dies by ramming the player (contact `self_damage`) spawn
  its pieces too? A: Yes. Any kill spawns them; only an escape does not.
- Q: Does `EnemySection` get `death_spawns` as well? A: No, out of scope.
- Q: Do pieces carry on with any of the parent's movement, or only follow their
  own behaviour? A: Pieces with no behaviour move at the parent's velocity from
  its last step. Pieces with a behaviour follow only their behaviour, because a
  path drives position directly.
- Q: How is "large splits into smaller" shown, with no smaller art? A: With a
  chain of the existing asteroid entries. Smaller art is out of scope.

**Notes:**

**Commits:**

**Manual verification:**

## Discovered

