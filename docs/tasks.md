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

### [todo] T-030 Make every behaviour absolute

**Review:** unreviewed

**Goal:**
Remove the spawn-relative behaviour space, so that every behaviour is absolute
and its paths play exactly as they are drawn in the behaviour editor. A map
spawn gets an offset vector that shifts the authored path, and formations get
an offset step, so that each member is shifted from the one before.

Context from the conversation: the editor canvas is already in screen
coordinates. Spawn-relative mode now only moves the first waypoint onto the
spawn point (`TransformBehaviour`). `behaviour_rotation` turns a path about its
first waypoint in either space, so rotation keeps working. The spawn-relative
behaviours today are `basic`, `in_n_out` and `in_n_out_bottom`.
`tiled_project/maps/stars.tmj` and `tiled_project/maps/desert.tmj` have two
enemy spawns each, so migrating the data is a small job.

**Requirements:**
- [ ] Behaviours have no spawn-relative space. Every behaviour plays its paths
  at the coordinates they were authored with.
- [ ] A map spawn has an offset vector that shifts the authored path.
- [ ] A formation can shift each member from the one before by an offset step.

**Open questions:**
- Q: Is the behaviour `edge` from T-029 removed? With absolute paths, the first
  waypoint already says where the enemy enters. Turning the formation offsets
  and extending the bounds could come from the direction of the first path
  segment instead.
- Q: How does a behaviour that starts with a Target or Hold state get a start
  point? For example, `chase_player` and `in_and_hold` have no first waypoint,
  and `desert.tmj` uses `chase_player`. The options discussed are to require
  the first state to be a path, or to treat the spawn offset as an absolute
  position when there is no path.
- Q: The Tiled object's x no longer places a top spawn, and the offset takes
  its place. Is it acceptable that the object's position only sets when the
  spawn triggers?
- Q: Is the offset step a separate spawn field, or a new built-in formation
  (for example `step`, with `step_x` and `step_y`)? Formations are already
  per-member offsets, and the built-in ones are a step times `spacing`, so a
  separate field could multiply with them.
- Q: What happens to `use_position`, `position_x` and `position_y` on
  `EnemySpawn` when the spawn has an offset?

**Notes:**

**Commits:**

**Manual verification:**

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

### [todo] T-034 Rename the spawn position properties to spawn_x and spawn_y

**Review:** unreviewed

**Goal:**
Change the `position_x` and `position_y` properties in Tiled objects to
`spawn_x` and `spawn_y`. This better explains what they do. When they are
present (or when a `use_spawn` toggle is enabled, if absence cannot be
checked), the enemy or formation spawns at those screen coordinates.

Context: `EnemySpawn` has `use_position`, `position_x` and `position_y` today
(see T-029). T-030 also has an open question about what happens to these
fields.

**Requirements:**
- [ ] The Tiled object properties `position_x` and `position_y` are renamed to
  `spawn_x` and `spawn_y`.
- [ ] When `spawn_x` and `spawn_y` are present (or `use_spawn` is enabled, if
  absence cannot be checked), the enemy or formation spawns at those screen
  coordinates.

**Open questions:**
- Q: Can the game tell that `spawn_x` and `spawn_y` are absent, or is a
  `use_spawn` toggle needed? If it is needed, does it replace `use_position`?
- Q: Are the existing maps migrated to the new names, and must the old names
  still load?
- Q: How does this relate to T-030, which may replace these fields with a
  spawn offset?

**Notes:**

**Commits:**

**Manual verification:**

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
which side to push the bounds out on. T-030's absolute paths would place the
formation from the path. The bounds could be pushed out by the offset length
on every side.

**Requirements:**
- [ ] The behaviour's `edge` enum is replaced by a number that rotates the
  formation by that amount.

**Out of scope:**
A rotation per behaviour step, interpolated between steps, so a formation can
spawn in one orientation and turn as it moves to the next step. This was
discussed and deliberately left for later.

**Open questions:**
- Q: Where is the formation placed, and which side do the bounds reach out on,
  once there is no edge? Does this task depend on T-030?
- Q: Is the number in degrees, and what is the field called?
- Q: What happens to existing behaviours that have `edge` set, for example
  `in_n_out_bottom`?

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-036 Add lifetime and screen-exit rules for escaping enemies

**Review:** unreviewed

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

**Requirements:**
- [ ] Each enemy's data defines a lifetime.
- [ ] An enemy that lives longer than its lifetime is escaped and removed.
- [ ] An enemy becomes active when it enters the visible screen area.
- [ ] An active enemy that leaves the visible area plus a small hardcoded
  border is escaped and removed.
- [ ] The lifetime keeps counting while the enemy is active.
- [ ] The lifetime is in milliseconds, and defaults to 15000 (15 seconds) when
  an enemy's data does not set it.
- [ ] These rules replace all existing enemy culling.
- [ ] All enemies follow these rules, including those spawned with no
  behaviour.

**Open questions:**
- Q: What units is the lifetime in, and what happens when an enemy's data does
  not set it? A: Milliseconds, with a default of 15 seconds.
- Q: Do these rules replace the current kill bounds (`ComponentBounds` with
  `BoundsBehaviour::Kill`) for enemies completely? A: Yes. They replace any
  existing enemy culling.
- Q: Does an enemy spawned with no behaviour, which moves with the map, follow
  the same rules? A: Yes. All enemies follow the same cull rules.

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-037 Set a path step's speed from its duration

**Review:** unreviewed

**Goal:**
Remove the speed value from the step, and instead the speed is determined by the
duration. If a path step takes 3 seconds, the enemy takes 3 seconds to travel to
the final point.

Context from the conversation: today a path step moves at its `speed`
(`BehaviourStateData::speed`), and the duration only decides when the step ends.
A duration of 0 ends the step at the last waypoint. A longer duration holds the
enemy at the last waypoint until it runs out. A shorter one cuts the path off
partway. The entry approach runs before the duration starts counting. The
editor preview's "seconds along the path" comes from length and speed.

**Requirements:**
- [ ] A path step has no speed value.
- [ ] A path step's speed is determined by its duration, so the enemy reaches
  the final waypoint when the duration runs out.

**Open questions:**
- Q: What does a path step with a duration of 0 do, now that 0 means "until it
  completes" and the path cannot complete without a speed? Is a duration
  required on path steps?
- Q: Does this apply only to path steps, or also to Target steps, which have a
  speed too?
- Q: The entry transition's speed falls back to "this state's" speed when it is
  0. What does it fall back to once a path step has no speed?
- Q: When the last step is a path, the enemy leaves its end at the step's speed.
  Is that speed now length ÷ duration?
- Q: Are the existing behaviours in `data/behaviour_database.json` converted
  (duration from length ÷ speed), or rewritten by hand? The file is disposable
  test data.

**Notes:**

**Commits:**

**Manual verification:**

## Discovered

- `SystemMap::Update` (`src/system_map.cpp`, the `m_cameraTop` line) has a
  `readability-math-missing-parentheses` clang-tidy finding that was there
  before the 2026-09-24 session.
