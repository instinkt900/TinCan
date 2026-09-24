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

### [todo] T-025 Let a spawn enter from any screen edge

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
- [ ] `tiled_project/tincan.tiled-project` gains a hand-authored enum `edge` with
  the values `Top`, `Bottom`, `Left`, `Right`, in that order, and `EnemySpawn`
  gains the members `edge` (default `Top`), `use_position` (bool, default
  `false`), `position_x` and `position_y` (floats, default 0). This change is
  allowed despite the profile's Compatibility section, because it is additive:
  no existing type, member or value is changed or reordered. The `edge` enum is
  not added to `tiled_project/extensions/game_data_enums.js`, because it does not
  come from the game data.
- [ ] The Tiled y position of the object still sets when the spawn triggers,
  whatever the edge, exactly as today.
- [ ] With `use_position` set, `position_x`/`position_y` is a point in screen
  pixels. The formation appears just outside the chosen edge (by `SpawnMargin`),
  level with that point: `Top` and `Bottom` use `position_x`, `Left` and `Right`
  use `position_y`. The other coordinate is ignored.
- [ ] With `use_position` unset, the point is the object's Tiled x and the middle
  of the screen height. A `Top` spawn with `use_position` unset behaves exactly
  as today, so the existing maps play unchanged.
- [ ] The edge adds a rotation (`Top` 0, and a quarter or half turn for the others,
  so that "moving down the screen" becomes "moving in from that edge"). It is
  added to the Tiled object rotation for the formation offsets, and to
  `behaviour_rotation` (plus `behaviour_rotation_step` per member) for the
  behaviour.
- [ ] A `World`-space behaviour's path is not rotated by the edge. The edge still
  places the formation and rotates its offsets, and `behaviour_rotation` still
  applies to it as today.
- [ ] Enemies spawned at the new positions are not killed straight away by their
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

**Commits:**

**Manual verification:**

### [todo] T-026 Fire enemy weapons only in chosen behaviour states

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
- [ ] `BehaviourStateData` has a `fire` flag, for every state type, read as an
  optional field that defaults to `true`, so existing behaviours fire as today.
  `to_json` writes it only when it is `false`.
- [ ] When a state starts, the enemy's weapons are switched on or off to match its
  `fire` flag: the root entity's weapon and the weapons of all its sections.
- [ ] After the last state ends, the weapons keep the last state's setting. An
  enemy with no behaviour fires as today.
- [ ] The behaviour editor shows the flag as a checkbox on each state, and saving a
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

## Discovered
