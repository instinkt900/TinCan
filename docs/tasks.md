# Tasks

The task list for TinCan. The process, the status values and the field
definitions are in the shared workflow, `/home/mcotton/Development/moth/.claude/tasks/workflow.md`.

- `/task-new <description>` adds a task.
- `/task-planning [IDs]` reviews tasks.
- `/task-session` implements reviewed tasks.

Session reports are in [sessions/](sessions/). `/task-planning` moves finished tasks
to [tasks-done.md](tasks-done.md).

## Tasks

### [done] T-040 Remove the passive pickups

**Review:** reviewed 2026-10-17

**Depends on:** none

**Goal:**
M1. The spec removes the passive pickups (fire rate, damage, burst), with their
code. Weapon levels replace them (T-039).

Context from planning: the passives are `ComponentPassives` and `PassiveType`
(`src/component_passives.h`), `PickupType::Passive` and `PickupData::value`
(`src/gamedata_pickup.h`), `PlayerPickupPassive` (`src/system_pickup.cpp`), and
the `Passives` multipliers in `SystemWeapon::Update` (`src/system_weapon.cpp`).
`CreatePlayer` (`src/game_world.cpp`) gives the player a `ComponentPassives`. No
entry in `data/pickup_database.json` is a passive.

**Requirements:**
- [x] `ComponentPassives`, `PassiveType` and their helpers are removed, with
  `src/component_passives.h`.
- [x] `PickupType` has no `Passive` value, and `PickupData` has no `value`.
- [x] `SystemWeapon::Update` fires each weapon with its own cooldown, burst,
  burst delay and damage, with no multipliers.
- [x] `data/pickup_database.json` still loads, and weapons fire as before.

**Out of scope:**
- Any change to how weapon pickups work (T-039).

**Open questions:**

**Notes:**
- The 50 ms floor on the burst delay in `SystemWeapon::Update` is kept. It is
  not a passive, and removing it would change how weapons fire.
- `PickupType` keeps its one value, `Weapon`, so the pickup data format does not
  change.

**Commits:**
- 06485fd refactor(T-040): remove the passive pickups

**Manual verification:**
1. Play the stars map. The player's main gun fires at the same rate and damage
   as before, and enemy weapons fire as before.
2. Collect a weapon pickup (kill a formation with a drop): it still adds the
   weapon.

### [done] T-039 Level up weapons through repeated pickups

**Review:** reviewed 2026-10-17

**Depends on:** T-040

**Goal:**
M1. Pickups today add a separate weapon each, and the player weapons are built
as an upgrade path (level 2 adds barrels to level 1, and so on). Because each
level is its own pickup, the player can pick up level 3 without level 2. The
spec instead gives each weapon 3 levels and one pickup: the first pickup adds
the weapon at level 1, each later pickup raises it by one, and a pickup for a
weapon at its last level gives 20 power.

The levels are defined in data as an ordered progression list rather than a
name-suffix convention, for example `"weapon_progressions": { "main_gun":
["main_gun_1", "main_gun_2", "main_gun_3"] }`. `WeaponData` stays single-level,
so enemy weapons are unchanged. Each level is a full definition, not a change to
the level before.

Context from planning: `AddWeapon` (`src/system_weapon.cpp`) already copies the
first weapon's timers so a new weapon fires on the same beat. The pickups are
`weapon_drop_01` (gives `player_weapon_01`) and `weapon_drop_02` (gives
`player_weapon_02`) in `data/pickup_database.json`, and the Tiled `pickup` enum
in `tiled_project/tincan.tiled-project` lists both. No map sets a `drop`, so no
map refers to them. Power has no cap yet; T-046 adds it.

This task is allowed to change `data/weapon_database_player.json`,
`data/pickup_database.json`, `data/player_database.json` and the `pickup` enum in
`tiled_project/tincan.tiled-project`.

**Requirements:**
- [x] The weapon data has ordered progressions that list a weapon's levels by
  weapon name. `WeaponData` itself stays single-level.
- [x] A weapon pickup's `name` and the player's starting weapon refer to a
  progression.
- [x] Picking up a weapon the player does not carry adds it at level 1.
- [x] Picking up a weapon the player carries raises it by one level.
- [x] Picking up a weapon that is at its last level gives the player 20 power.
  The 20 is a data value.
- [x] A carried weapon records its progression and current level.
- [x] A level-up rebuilds the weapon from the next level's data and keeps its
  timers, so it stays on the same beat as the other weapons.
- [x] The main gun progression has the spec's three levels as full definitions:
  2 barrels, 5 damage, every 100 ms; then 4 barrels; then 6 barrels at 7 damage.
- [x] The homing progression has one level, today's `player_weapon_homing_01`.
- [x] There is one pickup for the main gun and one for homing, and the Tiled
  `pickup` enum lists exactly the pickups in `data/pickup_database.json`.
- [x] The player starts with the main gun at level 1.

**Out of scope:**
- Homing levels 2 and 3, and the other weapons (M2).
- Pickup badges and the floating "WEAPON N" text (M2).
- The power cap (T-046).
- Enemy weapons.

**Open questions:**
- Q: What happens when the player picks up a weapon that is already at its last
  level? A: It gives 20 power (spec).
- Q: Do weapon levels survive the player dying? A: The main gun keeps its level;
  the other weapons drop as pickups (T-043).
- Q: Do enemy weapons stay plain single-level weapons? A: Yes.

**Notes:**
- Progressions are a new data category, `progressions`, in
  `data/weapon_database_player.json`: each entry is a list of weapon names, first
  level to last. (`weapon_progressions` cannot be a category key, because keys
  are matched to `GameDataCategory` names.)
- The player weapons are renamed to `main_gun_1` to `main_gun_3` and `homing_1`,
  and the pickups to `pickup_main_gun` and `pickup_homing`. The Tiled `pickup`
  enum and the `PickupSpawn` default follow. No map used the old names.
- `SystemWeapon::AddWeaponLevels` adds or raises a weapon by any number of
  levels and returns how many did not fit, so T-043's death drops can use it.
  A carried weapon keeps its progression in `Weapon::m_progression` and its
  level as an index in `Weapon::m_level`.
- The 20 power is `max_level_power` in the player data. Power has no cap until
  T-046.
- Main gun level 2 is level 1's barrels plus the old level 2's (±12); level 3
  adds the old level 3's (±20, 4 px forward) at 7 damage.
- The player editor (F2) still adds weapons by weapon name. A weapon added there
  has no progression, so a pickup of the same progression adds a second copy.
  It is a debug tool and was left alone; M2 plans a debug tool for weapon levels.

**Commits:**
- 5336de6 feat(T-039): level up weapons through repeated pickups

**Manual verification:**
1. Play the stars map. The ship fires the main gun with 2 barrels.
2. Give a formation a `pickup_main_gun` drop in Tiled (or use the T-050 test
   map). Collect it: the main gun fires 4 barrels. Collect another: 6 barrels,
   with more damage. The new barrels fire in step with the old ones.
3. Collect a fourth main gun pickup: the barrels do not change, and the power
   readout goes up by 20.
4. Collect a `pickup_homing` drop: homing missiles join the main gun. Collect
   another: power goes up by 20.
5. Open `tiled_project/tincan.tiled-project` in Tiled: the `pickup` enum lists
   `none`, `pickup_main_gun` and `pickup_homing`.

### [done] T-041 Play sound effects

**Review:** reviewed 2026-10-17

**Depends on:** none

**Goal:**
M1. The game has no sound. The spec's Feel table needs a sound for nearly every
event, and the roadmap counts a feature as done only with its sound, even a
placeholder. This task adds sound effect playback first, so that the tasks after
it can each add their own sounds.

Context from planning: moth_toolkit has `moth_audio` 0.1.0 (miniaudio), used by
`examples/audio_demo` with `self.requires("moth_audio/[~0.1]")`. The user adds
`moth_audio/[~0.1]` to `conanfile.py` and `moth::audio` to `CMakeLists.txt`, and
runs `conan install`, before the session; the session does not change either
file. If the dependency is not there, the task is blocked. A session cannot
download sounds, so the placeholders are generated.

**Requirements:**
- [x] The game uses `moth_audio` from the dependency the user added.
- [x] Sounds are named entries in a new `data/sound_database.json`, loaded
  through `GameData`, each with a file and a volume.
- [x] Game code plays a sound by its data entry, and a missing entry or file is
  logged once and plays nothing.
- [x] Many copies of a sound can play at once, so 50 enemies dying together do
  not cut each other off, and the number playing at once has a limit.
- [x] A script in `tools/` generates the placeholder sounds as WAV files in
  `assets/sounds/`, and the generated files are committed.
- [x] Placeholder sounds play for: the player's shots (main gun and homing each
  their own), an enemy shot, a hit on an enemy, an absorb, an enemy death and a
  pickup.

**Out of scope:**
- Music (M4).
- Volume settings (M6).
- Sounds for events later tasks add (player death, extra life, power release and
  empty): those tasks add them.

**Open questions:**
- Q: Who adds the `moth_audio` dependency? A: The user, before the session,
  with `conan install`.
- Q: Where do placeholder sounds come from? A: A script generates them.

**Notes:**
- `SoundPlayer` (`src/sound_player.h`) owns the `moth::audio::AudioEngine` and is
  owned by `GameWorld`. Code asks for a sound with `SystemSound::Play(registry,
  name)`, which queues it on a `ComponentSoundQueue` entity; `SystemSound::Update`
  plays the queue at the end of the world update. That lets code with only the
  registry, such as `SystemDamage::ApplyDamage`, ask for sounds.
- The voice limit is per sound: each entry's `voices` (default 8). When every
  voice is busy, the oldest-started one is restarted. A sound asked for many
  times in one frame plays once.
- A missing entry or file is logged once as an error, and the sound then stays
  silent. If the audio engine cannot start, that is logged as an error and the
  game plays without sound.
- Weapons name their shot sound in a new optional `sound` field. Assumption:
  adding an optional field to `data/weapon_database_enemy.json` (and the player
  weapons) is allowed, because the file still loads with or without it and the
  task needs a per-weapon sound. The other sounds use fixed entry names:
  `hit`, `absorb`, `enemy_death` and `pickup`.
- `EnemyTag` is never set on enemies, so the hit and death sounds play for
  entities on `Team::Enemy` (through `GetEntityDetails`, so sections count).
- `src/sound_player.cpp` and `src/system_sound.cpp` are added to `SOURCES` in
  `CMakeLists.txt`, as the profile's rules require. The dependency lines were
  already there.
- The launch check was also run for 20 seconds, so that enemies fire during it.
  It passed.

**Commits:**
- 4cd0521 feat(T-041): play sound effects

**Manual verification:**
1. Play the stars map with sound on. Holding fire plays the main gun's shot
   sound; it stays clean, not cut off, at the full fire rate.
2. Enemy shots make a sound. Hitting an enemy clicks, and killing one plays an
   explosion. Killing a whole formation at once is not deafening.
3. Absorb a bullet of the shield's colour: a rising chirp plays.
4. Collect a pickup: a short arpeggio plays. A homing pickup's missiles have
   their own shot sound.
5. Rename `assets/sounds/hit.wav` temporarily: the game logs one error for it
   and carries on without that sound. Restore it.
6. To regenerate the placeholders, run `python3 tools/make_placeholder_sounds.py`
   from the repository root.

### [todo] T-042 Destroy the ship with one hit

**Review:** reviewed 2026-10-17

**Depends on:** T-041

**Goal:**
M1. As in Ikaruga, the ship has no health: one bullet that gets through the
shield, or one contact with an enemy, destroys it (spec, Death and lives).

Context from planning: the player has a `ComponentHealth` of 100 from
`data/player_database.json`, and damage goes through `SystemDamage::ApplyDamage`
(`src/system_damage.cpp`), which also shakes the camera on every death.
Projectiles hit in `SystemProjectile`, contact is `SystemContact`, and only the
asteroids have `contact` data, so touching a danger enemy does nothing today.
The player's hitbox is its 5 px body; the 45 px shield is a separate entity.
There is no hit-stop yet. Until T-022, a destroyed ship does not come back, as
today.

**Requirements:**
- [ ] The player has no health. `health` is gone from `PlayerData` and
  `data/player_database.json`.
- [ ] An enemy bullet that reaches the player's hitbox destroys the ship.
- [ ] Contact with any enemy destroys the ship, including enemies with no
  `contact` data. What contact does to the enemy stays as its data says.
- [ ] The player's death has a big explosion, a strong camera shake, a 100 ms
  hit-stop and a death sound.
- [ ] Hit-stop freezes the game update for a set time, so other events can use it
  (T-049). The scroll and the enemies carry on after it.
- [ ] The health line is gone from the debug text in `GameLayer`.

**Out of scope:**
- Lives and respawn (T-022).
- Enemy health and damage.
- The camera shake on ordinary enemy deaths (T-049).

**Open questions:**

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-022 Give the player lives and a respawn

**Review:** reviewed 2026-10-17

**Depends on:** T-039, T-042

**Goal:**
M1. The run starts with 3 lives and holds at most 5. When the ship is destroyed,
it comes back after a short time, safe for a moment, so that dying costs time
and weapons but can be recovered (spec, Death and respawn).

Context from planning: today a destroyed player entity is removed by
`SystemLifetime` and nothing creates a new one; `SystemPickup` already treats no
player as a normal state. `CreatePlayer` (`src/game_world.cpp`) places the ship
at the bottom centre, 160 px up from the bottom. The level intro in M4 (T-024)
flies the ship in the same way, so the fly-in should be reusable.

**Requirements:**
- [ ] The player starts with 3 lives and holds at most 5. Both are data values.
- [ ] A destroyed ship costs one life. The scroll and the enemies carry on.
- [ ] 1.5 seconds after the death, the ship flies in from below the bottom
  centre of the screen to its start point. The player has no control during the
  fly-in.
- [ ] When it arrives, the player has control, and the ship is safe for 3
  seconds: enemy bullets and contact do not destroy it. It blinks while safe, and
  it can fire and absorb.
- [ ] On respawn, the main gun keeps its level, every other weapon is gone, and
  power is 0.
- [ ] With no lives left, the ship does not come back. (T-045 adds the game
  over.)
- [ ] The debug text in `GameLayer` shows the lives until T-047 replaces it.
- [ ] The delays, the fly-in time and the safe time are data values.

**Out of scope:**
- Death drops (T-043).
- Score and extra lives (T-044).
- The game over (T-045).

**Open questions:**
- Q: How many lives, and is that data? A: 3 at the start, at most 5, in data.
- Q: Respawn where, after what delay, with what safe time? A: Fly-in from the
  bottom centre after 1.5 s, then 3 s safe with blinking.
- Q: Does the player control the ship during the fly-in? A: No, only once it
  arrives. The safe time starts then.
- Q: What is kept on death? A: The main gun's level and the score. The other
  weapons drop (T-043), and power goes to 0.
- Q: Can lives be gained? A: Through score only (T-044).

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-043 Drop the carried weapons on death

**Review:** reviewed 2026-10-17

**Depends on:** T-022

**Goal:**
M1. When the ship is destroyed, every weapon it carries except the main gun
drops as its own pickup, so the player can win them back (spec, Death and
respawn).

A dropped pickup holds as many levels as its weapon had, and adds them on top
of whatever the ship carries when it is collected. A dropped spread 2, collected
after a new spread pickup (spread 1), gives spread 3. Each level past the last
gives 20 power, as a pickup at the last level does.

Context from planning: pickups are made by `SystemPickup::CreatePickup` and drift
at the map's scroll speed. A weapon's pickup is the pickup entry whose `name` is
its progression (T-039).

**Requirements:**
- [ ] On death, each carried weapon except the main gun drops as one pickup, using
  that weapon's pickup look, spread out from where the ship died.
- [ ] A dropped pickup adds as many levels of its weapon as the weapon had when
  the ship died, on top of the ship's current level of it, or from none.
- [ ] Each level that would go past the weapon's last level gives 20 power
  instead.
- [ ] Dropped pickups drift down with the scroll and are removed after 8 seconds
  (a data value), or when they leave the bottom of the screen.
- [ ] A weapon with no pickup entry is logged and not dropped.

**Out of scope:**
- Pickup badges and floating text (M2).
- The collect radius (M2).

**Open questions:**
- Q: Does a dropped pickup set the weapon to its old level, or add levels? A: It
  adds as many levels as the weapon had, on top of what the ship carries. Levels
  past the last give 20 power each.

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-044 Score points and earn extra lives

**Review:** reviewed 2026-10-17

**Depends on:** T-022

**Goal:**
M1. Every enemy has a score value in its data, and killing it adds that value,
whatever killed it. Every 100,000 points gives a life (spec, Score).

Context from planning: `SystemLifetime` tells kills from escapes through
`EscapedTag`, and death spawns are spawned there. The spec's starting values are
by enemy role; today's enemies map to them by the spec's Art column. This task is
allowed to change `data/enemy_database.json` and `data/player_database.json`.

**Requirements:**
- [ ] `EnemyData` has a `score`, 0 when not set.
- [ ] Killing an enemy by any means (shots, the power weapon, ramming the ship)
  adds its score. An escaped enemy adds nothing. Each death spawn piece scores
  its own value.
- [ ] Starting values in `data/enemy_database.json`: danger_1 100, danger_2 300,
  danger_3 300, danger_4 1000, danger_5 200, danger_6 5000, and each asteroid
  200.
- [ ] Losing a life keeps the score.
- [ ] Every 100,000 points (a data value in the player data) gives a life. A
  life earned at the most lives is lost. The progress towards the next life is
  kept separately from the score, so T-045 can reset it.
- [ ] An extra life shows "1UP" floating up from the ship for 1 second, with a
  sound. The floating text can be reused for other text (M2).
- [ ] The debug text in `GameLayer` shows the score until T-047 replaces it.

**Out of scope:**
- Boss sections and boss bonuses (M5).
- The score display (T-047).
- Resetting the score on a game over (T-045).

**Open questions:**

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-045 Show the game over overlay and retry the level

**Review:** reviewed 2026-10-17

**Depends on:** T-044

**Goal:**
M1. When the last life is lost, a game over overlay shows over the game.
"GAME OVER" shows at once, and a black background element in the overlay fades
in over 2 seconds while the game carries on behind it. When the fade ends, the
game behind stops, and the overlay waits for a key: retry or quit (spec, Menus
and screens).

Retry restarts the level as a game over does: 3 lives, the weapons, levels and
power the player had when they entered the level, and the score and the progress
towards the next extra life back to zero (spec, Run structure).

Context from planning: the spec makes every screen a moth::ui layout, and TinCan
has none yet; `src/main.cpp` has the font project loading commented out. The game
is one `GameLayer` holding a `GameWorld`. In M1 the level is the map the game
loaded, entered at the start of the run. Escape is bound to `Quit` and exits the
game today; M6 changes that.

The session writes the layout file by hand. A plain placeholder is fine: the art
pass restyles it later.

**Requirements:**
- [ ] When the last life is lost, the overlay appears after the 1.5 s respawn
  delay, where the ship would otherwise fly in.
- [ ] The overlay is a moth::ui layout, in its own layer over the game. Its
  "GAME OVER" text shows at once. A full-screen black element fades in from
  clear to opaque over 2 seconds, as an animation in the layout.
- [ ] The game keeps running behind the overlay during the fade, and stops when
  the fade ends.
- [ ] After the fade, Fire retries and Quit exits the game. Before the fade ends,
  neither does anything. Pause does nothing while the overlay shows.
- [ ] Retry removes the overlay and restarts the map from its start: every enemy,
  projectile and pickup is gone, the scroll starts again, and the player has 3
  lives and the weapons, levels and power they had when the level started.
- [ ] Retry sets the score and the progress towards the next extra life to 0.
- [ ] The state the player had when the level started is recorded when the level
  starts, so M6 can carry it from the level before.

**Out of scope:**
- The high score table on the overlay (M7).
- Quit going to the main menu, and the pause menu's Restart (M6).
- Saving (M6).

**Open questions:**
- Q: What does the game over look like in M1? A: An overlay with "GAME OVER" and
  a black background that fades in over 2 seconds while the game carries on.
  When the fade ends the game stops and the overlay waits to retry or quit.
- Q: Who makes the layout? A: The session, by hand, as a placeholder.

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-046 Rework the power weapon

**Review:** reviewed 2026-10-17

**Depends on:** T-041

**Goal:**
M1. Power is stored up to 100, as 10 segments of 10. Pressing the power button
spends every full segment, and each one fires 4 homing missiles. Each absorbed
bullet adds 1 power (spec, Shield and affinity, Power weapon).

Context from planning: today `SystemShield` adds the absorbed bullet's damage as
power, with no cap. `SystemPowerWeapon::InitWeapon` sets the projectile, power
per projectile, speed and damage in code, and fires one fan of up to 10° per shot
every frame the button is held. `UpdateTargetingRandom`
(`src/system_targeting.cpp`) already picks a random target. This task is allowed
to change `data/player_database.json`.

**Requirements:**
- [ ] Each absorbed bullet adds 1 power.
- [ ] Power is capped at 100. Power over the cap is lost, from any source,
  including the 20 power from weapon pickups (T-039, T-043).
- [ ] The power weapon's values are in the player data: the cap, the segment
  size (10), missiles per segment (4), missile damage (50), the projectile and
  its speed.
- [ ] The power weapon fires on the press of its button, not while it is held.
- [ ] A press spends every full segment and leaves the partial one. Each spent
  segment fires 4 missiles that fan out from the ship and each seek a random
  enemy.
- [ ] A press with no full segment does nothing but play a short "empty" sound.
- [ ] A release has a screen flash, a short camera shake and its own sound. It
  does not clear bullets and does not make the ship safe.

**Out of scope:**
- The power meter (T-047).
- The absorb effect (T-049).

**Open questions:**

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-047 Show the on-screen display

**Review:** reviewed 2026-10-17

**Depends on:** T-044, T-046

**Goal:**
M1. The on-screen display shows only the basics: lives top left, score top
right, and the power meter bottom left (spec, On-screen display).

Context from planning: `GameLayer::Draw` draws the world into its own image,
scales it into the window, then draws debug text lines for health and power.
The display is drawn inside the play field, so it scales with the world and
stays on the map whatever the window's shape.

**Requirements:**
- [ ] Top left: the lives as ship icons.
- [ ] Top right: the score.
- [ ] Bottom left: the power meter, 10 segments; full segments glow.
- [ ] When an extra life is earned, the new icon flashes.
- [ ] The display is drawn in the play field, over the world and under the game
  over overlay, and scales with it.
- [ ] The debug text lines in `GameLayer` are removed.

**Out of scope:**
- Any other display (spec: nothing else).

**Open questions:**
- Q: Inside the play field or at the window edges? A: Inside the play field.

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-048 Add feel to shots and hits

**Review:** reviewed 2026-10-17

**Depends on:** T-041

**Goal:**
M1. The spec's Feel table: when the player fires, a muzzle flash and the shot
sound; when a shot hits an enemy, the enemy flashes white and a spark shows at
the hit point.

Context from planning: an enemy hit already sets the sprite's flash time in
`SystemDamage::ApplyDamage`. One-shot sprites are made with
`SystemDrawable::CreateOneShotSprite`. Placeholder art is fine.

**Requirements:**
- [ ] Each player weapon shows a muzzle flash at its barrels when it fires.
- [ ] A player shot that hits an enemy shows a spark at the hit point.
- [ ] A hit enemy flashes white.
- [ ] The flash and spark sprites are named in data.

**Out of scope:**
- A spark and sprite per weapon (M2).

**Open questions:**

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-049 Add feel to absorbs and enemy deaths

**Review:** reviewed 2026-10-17

**Depends on:** T-042, T-047

**Goal:**
M1. The spec's Feel table: an absorbed bullet pulls into the shield, the shield
pulses, and the meter ticks. An enemy death has an explosion sized to the enemy
and holds up with 50 on screen. A large enemy's death has a bigger explosion, a
shake and a 50 ms hit-stop. Shake and hit-stop are used only where the table
says.

Context from planning: `SystemDamage::ApplyDamage` shakes the camera on every
death today. The hit-stop comes from T-042. This task is allowed to change
`data/enemy_database.json`.

**Requirements:**
- [ ] An absorbed bullet moves into the centre of the shield over a short time
  before it is removed, and does no harm while it does.
- [ ] The shield pulses on each absorb, and the power meter's current segment
  ticks.
- [ ] An enemy's death explosion is sized to the enemy.
- [ ] Ordinary enemy deaths do not shake the camera.
- [ ] An enemy marked large in its data has a bigger explosion, a camera shake
  and a 50 ms hit-stop when it dies. danger_6 is marked large.
- [ ] 50 enemies dying together stays smooth.

**Out of scope:**
- Boss deaths (M5).

**Open questions:**

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-050 Add a test map for the core fight

**Review:** reviewed 2026-10-17

**Depends on:** T-039, T-044

**Goal:**
M1. A test map with a few waves, for replaying the fight while M1 to M3 are
built (roadmap, M1).

Context from planning: the game loads `tiled_project/maps/stars.tmj` by a path in
`GameWorld`'s constructor. Pickups come only from spawn drops until M4 adds pickup
spawns.

**Requirements:**
- [ ] A new map, `tiled_project/maps/test.tmj`, about 2 minutes long, on the stars
  tileset, using only existing enemies, behaviours, formations and pickups.
- [ ] It has light and dark fodder formations, gunners from both sides, a few
  enemies to absorb from, an asteroid chain, and formation drops that give main
  gun levels and homing.
- [ ] The game takes a `--map <path>` command-line option. With none, it loads
  the stars map, as today.
- [ ] Both existing maps still load and play as before.

**Out of scope:**
- New enemies, behaviours or pickups.

**Open questions:**
- Q: How does the game pick the test map? A: A `--map` option, with the stars
  map as the default.

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-023 Pause the map scroll

**Review:** unreviewed

**Goal:**
M4. Add map scroll pause support. Some areas need the scroll to stop until a
fight is over, for example set pieces and boss fights.

From the spec (Levels, Map objects): a scroll pause is an object placed in the
map. It stops the scroll until every enemy spawned since the last pause is gone,
or a boss dies. While the scroll is paused the background still moves (the
parallax stars), so the screen never looks frozen. Spawns below the pause wait
for it to release.

**Requirements:**
- [ ] The map scroll can be paused until a trigger releases it.

**Open questions:**
- Q: How is a pause authored in the map data: a point on the map, a spawn, or
  something else? A: An object placed in the map (spec).
- Q: What kinds of trigger release a pause? A: Every enemy spawned since the last
  pause is gone, or a boss dies (spec). No timer.
- Q: Does anything else stop while the scroll is paused? A: The background keeps
  moving; spawns below the pause wait for it (spec).
- Q: Can a map hold several pauses, and can a pause be skipped?

**Notes:**

**Commits:**

**Manual verification:**

### [todo] T-024 Add intro and exit animations

**Review:** unreviewed

**Goal:**
M4. Add the level intro and exit (spec, Level intro and exit).

- Intro: the ship flies in from below the screen to its start point over 1.5
  seconds while the level name shows. The player gets control when it arrives.
- Exit: after a boss dies, the player has 3 seconds to collect the drops, then
  loses control and the ship flies off the top of the screen. "LEVEL COMPLETE"
  shows for 3 seconds, then the next level starts.

The respawn fly-in (T-022) works the same way as the intro.

**Requirements:**
- [ ] A map plays an intro, with the player flying in.
- [ ] A map plays an exit, with the player flying out.

**Open questions:**
- Q: Where does an intro or exit animation start and end? A: Each level (spec).
- Q: Is the animation authored in data, or is it code?
- Q: What does the player control during the animation, and what happens to the
  scroll, the spawning and the enemies while it plays? A: No control during
  either. The rest is not yet decided.
- Q: Does it also cover death and respawn? A: The respawn fly-in is T-022; the
  intro should reuse it.

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

### [todo] T-051 Add the missing Tiled enum values

**Review:** unreviewed

**Goal:**
M4. The Tiled `enemy_type` enum lacks `enemy_basic`, which `desert.tmj` spawns,
and the `behaviour` enum lacks `drift_out`. (Found in session 2026-09-25.)

**Requirements:**
- [ ] The Tiled enums list every enemy and behaviour the maps and data use.

**Open questions:**

**Notes:**

**Commits:**

**Manual verification:**

## Discovered

- `tools/smoke_launch.sh` failed once in eight runs: the game did not exit
  within 15 seconds of the close request and was killed (exit 137), with no
  shutdown lines and no warnings in the log. It passed every other time.
  (Session 2026-09-25.)
