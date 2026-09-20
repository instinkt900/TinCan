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

### [todo] T-020 Add a formation editor window

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
- [ ] F2 toggles editor mode, which shows both the Behaviour Editor and the new
      Formation Editor as separate ImGui windows.
- [ ] Editor mode pauses the map update exactly as the behaviour editor does
      today (`src/game_world.cpp:53`).
- [ ] The most recently focused of the two windows owns the world view: its
      overlay is drawn there and the mouse edits its shape. The Behaviour Editor
      is the active one when editor mode opens.
- [ ] Clicking in the world view does not change which window is active.
- [ ] The behaviour editor keeps its current drawing and mouse editing, including
      its guides, while it is the active window.
- [ ] The Formation Editor has a combo over the formations the game knows: the
      entries of `data/formation_database.json` and the built-in `line`, `column`,
      `v` and `grid`.
- [ ] A built-in formation can be selected and previewed, using `spacing` rather
      than scale, but has no slots to edit and cannot be saved.
- [ ] A database formation's slots can be created and edited: click in the world
      view to add a slot, drag to move one, right-click to remove one, at the
      preview scale, stored in unit space.
- [ ] The panel lists the slots as editable unit-space number pairs with a remove
      button and an add button, as the waypoint list does.
- [ ] The preview controls are a count, a per-axis scale and a rotation in
      degrees, and the preview is built with `GetFormationOffsets` from T-019.
- [ ] Slots within the count are drawn differently from the slots beyond it, so a
      shape can be judged at the count it will be spawned with.
- [ ] A `Save as` name and a Save button write the formation back to
      `data/formation_database.json`, replacing an entry of that name or adding a
      new one, mirroring `SaveBehaviour`
      (`src/system_behaviour_editor.cpp:38`). Save is disabled for a built-in.
- [ ] Any new `.cpp` is added to `SOURCES` in `CMakeLists.txt`.

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

**Commits:**

**Manual verification:**

### [todo] T-021 Fill the counted slots with the enemy's sprite

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
- [ ] The Formation Editor has its own combo over the enemy database, independent
      of the behaviour editor's.
- [ ] Each slot within the count draws that enemy's sprite, through the function
      exposed in T-019.
- [ ] Slots beyond the count keep their marker, so the whole shape is still
      visible while a smaller count is previewed.
- [ ] No entities are created for the preview.
- [ ] With no enemy chosen, the slots fall back to the markers from T-020 rather
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

**Commits:**

**Manual verification:**

## Discovered
