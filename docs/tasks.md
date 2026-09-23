# Tasks

The task list for TinCan. The process, the status values and the field
definitions are in the shared workflow, `/home/mcotton/Development/moth/.claude/tasks/workflow.md`.

- `/task-new <description>` adds a task.
- `/task-planning [IDs]` reviews tasks.
- `/task-session` implements reviewed tasks.

Session reports are in [sessions/](sessions/). `/task-planning` moves finished tasks
to [tasks-done.md](tasks-done.md).

## Tasks

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

**Review:** unreviewed

**Goal:**
Enemies can only enter from the top of the screen. A spawn is placed at its Tiled
position when that position comes within `SpawnMargin` of the top of the screen,
and a `SpawnRelative` behaviour is anchored there. A `World` behaviour can start
elsewhere, but it throws away the formation offsets, so every member of a
formation follows the same path. The example wanted: a line of 5 enemies that
flies in from the bottom of the screen, hovers, fires and then moves off screen.

Add an `edge` property (`Top`, `Bottom`, `Left`, `Right`) to `EnemySpawn`. The
Tiled y position still sets when the spawn triggers, and the edge sets where the
formation appears. The edge also rotates both the formation offsets and the
behaviour, so every behaviour is authored once as "enter from the top" and works
from any edge.

**Requirements:**
- [ ] `EnemySpawn` has an `edge` property with `Top`, `Bottom`, `Left` and
  `Right`. `Top` is the default and behaves as today.
- [ ] The Tiled y position still sets when the spawn triggers, whatever the edge.
- [ ] The formation appears just outside the chosen edge.
- [ ] The edge rotates the formation offsets and the behaviour, so a behaviour
  authored as entering from the top enters from the chosen edge.
- [ ] Enemies spawned at the new positions are not killed straight away by their
  bounds.

**Open questions:**
- Q: For `Left` and `Right`, what sets the position down the screen? The Tiled y
  is already used for timing, so this needs another property.
- Q: How does the edge rotation combine with an existing `behaviour_rotation`
  and the spawn's own rotation: added together, or does one replace the other?

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-026 Fire enemy weapons only in chosen behaviour states

**Review:** unreviewed

**Goal:**
Enemy weapons fire from the moment the enemy spawns until it dies, whatever its
behaviour is doing. It is active from `InitWeapon`, and nothing turns it off. So
an enemy cannot fly in quietly, hover and shoot, and then leave quietly. It also
fires while it is still off screen, which gives shots the player cannot see
coming.

Let a behaviour state say whether the enemy fires during it, for example a `fire`
flag on each state. The behaviour system turns the weapon on when a state that
fires starts, and off when it ends.

**Requirements:**
- [ ] A behaviour state can say whether the enemy's weapons fire during it.
- [ ] Enemy weapons fire only during states that allow it.

**Open questions:**
- Q: What is the default for a state that does not say: fire (as today, so
  existing behaviours are unchanged) or do not fire?
- Q: Do the weapons on enemy sections follow the root's behaviour state too?
- Q: Should the behaviour editor show and edit the flag?

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
