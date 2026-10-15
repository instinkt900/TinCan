# TinCan Roadmap

The order in which to build the [design](design.md). Each milestone ends with
something playable. Finish a milestone before starting the next one.

## At a glance

| #  | Milestone                                    | Result                                | Status      |
| -- | -------------------------------------------- | ------------------------------------- | ----------- |
| M1 | [Core combat](#m1-core-combat)               | One fight feels good and has stakes   | In progress |
| M2 | [Weapon roster](#m2-weapon-roster)           | Five weapons, each worth picking up   | Not started |
| M3 | [Enemy roster](#m3-enemy-roster)             | Eight enemies, each with its own job  | Not started |
| M4 | [One complete level](#m4-one-complete-level) | A level plays from start to end       | Not started |
| M5 | [Sectioned bosses](#m5-sectioned-bosses)     | The level ends in a real boss fight   | Not started |
| M6 | [Run structure](#m6-run-structure)           | A run from the menu to the last level | Not started |
| M7 | [Content](#m7-content)                       | The full game, in rough form          | Not started |
| M8 | [Polish](#m8-polish)                         | The full game, finished               | Not started |

## Rules

- **Depth before breadth.** One level that plays well comes before five that do
  not. The core of the game cannot be built without some content, so M2 and M3
  make a starting set of weapons and enemies: enough to build the core from, not
  the final set. Once the core exists, M7 refines and expands them.
- **Feel is part of done.** A feature is not finished until it has its hit
  flash, sound or effect, even a placeholder one.
- **Decide before building.** Settle a milestone's open decisions at its start.
- **Park the rest.** An idea outside the current milestone goes in the task list
  with its milestone named in the goal, not into the code.

## M1 Core combat

Goal: the moment-to-moment fight is fun, and dying matters.

Today the player has 100 health, a shield that absorbs bullets of its own
affinity into power, and a power weapon that spends all stored power on one fan
of homing shots. Each weapon level is its own pickup, and passive pickups raise
fire rate, damage and burst. Enemies flash when hit, and the camera can shake.
There is no on-screen display, and a death ends the game.

Decide first:
- Player health, or one bullet through the shield kills (Ikaruga). This sets
  how hard every enemy in M3 can hit.
- The power weapon: what it does, how it is triggered, and what it costs (all
  stored power, or fixed amounts). Whether it also clears bullets or gives a
  moment of safety.
- Do passive pickups stay alongside weapon levels?
- Lives: how many, where and how the ship respawns, and how long it is safe
  after respawning (T-022).
- What a pickup does for a weapon already at its last level (T-039).
- What the player must see on screen: lives, power, health, weapon levels.

Build:
- Weapon levels through repeated pickups (T-039).
- Lives, and respawn with a short safe time (T-022).
- Death drops that give weapons back at their levels.
- The power weapon, as decided.
- Passive pickups kept, changed or removed, as decided.
- A minimal on-screen display for what the player must see.
- First feedback pass, each as its own task:
  - Player hit and player death.
  - Enemy explosions that hold up in large numbers.
  - Absorb effect, so the power coming in is clear.
  - Pickup and level-up effects.
  - Power weapon effect, so it feels big.
- A test map with a few waves, for replaying the fight.

Done when: a test map with a few waves is fun to replay, and losing a life hurts
but can be recovered.

## M2 Weapon roster

Goal: a starting set of five weapons, each with levels, and each the best
choice somewhere.

Today there is the main gun (three levels) and homing missiles (one level).
Weapon levels (T-039) come in M1, so this milestone is about variety.

Decide first:
- The five weapons, and the job of each: what it is good against and what it is
  weak against. A starting list to argue with:

  | Weapon            | Job                                      | Needs                  |
  | ----------------- | ---------------------------------------- | ---------------------- |
  | Main gun          | Steady forward damage; the default       | Exists                 |
  | Homing missiles   | Clears scattered weak enemies            | More levels            |
  | Spread shot       | Covers the screen, weak at range         | Short-range shots      |
  | Laser             | Cuts through lines of enemies            | Piercing shots         |
  | Side or rear guns | Covers enemies from the sides and behind | Exists (barrel angles) |

- How many levels each weapon has, and what a level changes: more barrels, more
  damage, faster fire, or a new effect.
- How the weapons combine, since the ship carries several at once: can the
  player hold all five, and does that stay a challenge?
- How the player tells pickups apart, before and after collecting them.

Build:
- The new shot types the list needs (for example piercing and short-range
  shots), each as its own task.
- Each weapon in data, with all its levels.
- A pickup per weapon, with its own look.
- Feel for each weapon: its own shot sprite, hit effect and placeholder sound.
- A test map or debug tool that gives any weapon at any level.

Done when: each weapon has a situation where it is the best pick, and each
level-up is a clear step up.

## M3 Enemy roster

Goal: a starting set of eight enemy types, each with a clear job in a wave, so
M4 has enough to build a level from.

Today there are the danger_1 to danger_6 sprites (light and dark), two enemy
weapons, and the asteroid chain that splits on death (T-027). An enemy type
counts once; its light and dark variants are free.

Decide first:
- The eight enemies, and the job of each. A starting list to argue with:

  | Enemy    | Job                                                   | Needs                   |
  | -------- | ----------------------------------------------------- | ----------------------- |
  | Fodder   | Weak, no fire, comes in large formations              | Exists                  |
  | Gunner   | Fires straight ahead in light or dark                 | Exists                  |
  | Aimer    | Fires bursts at the player                            | Exists (tracking)       |
  | Sprayer  | Fires fans or rings of bullets; the main power source | Spirals are new         |
  | Chaser   | Flies at the player and rams                          | Exists (chase, contact) |
  | Heavy    | Slow and tough, fires a lot; a mid-wave anchor        | Tuning, maybe sections  |
  | Splitter | Breaks into smaller enemies when killed               | Exists (T-027)          |
  | Carrier  | Releases fighters while it is alive                   | Spawning while alive    |

  Others to weigh: turrets fixed to the map, mines that burst into bullets, an
  enemy that switches affinity.
- How affinity is used across the roster: one affinity per enemy, or enemies
  that fire both.
- Where the art comes from, since there are six enemy sprites today.
- Which enemies drop pickups, and which weapons each drop gives.

Build:
- The new enemy features the list needs (for example spiral patterns and
  spawning while alive), each as its own task.
- Each enemy in data, with its weapon, behaviours and formations.
- Feel for each enemy: hit flash, death effect and placeholder sound.
- A test map with a wave of each enemy.

Done when: each enemy reads at a glance, asks the player for something
different, and is tuned against the M2 weapons.

## M4 One complete level

Goal: one level that shows the whole game in small.

Today waves are Tiled point objects in the map, each with an enemy, formation,
behaviour and drop. The game loads the stars map; the desert map also exists.
The map always scrolls, and there is no dialog, intro, exit or sound.

Decide first:
- Which look the first level uses, and how long it lasts (a few minutes is a
  good start).
- The level's outline, before any waves are placed. A starting shape:

  | Beat      | What happens                                         |
  | --------- | ---------------------------------------------------- |
  | Intro     | The ship flies in; a line or two of dialog           |
  | Opening   | Fodder and gunners; the first weapon pickup          |
  | Build-up  | New enemies one at a time, more pickups              |
  | Set piece | A scroll pause with a heavy or carrier fight         |
  | Push      | Dense mixed waves; the player is at full power       |
  | Boss      | Scroll pause, dialog, the boss                       |
  | Exit      | The ship flies out; level complete                   |

- Where pickups go, so the weapons grow at a good rate across the level.
- Scroll pauses: how they are placed in the map and what releases them (T-023).
- Dialog: how it is shown, how it is placed in the map, whether the action
  continues while it shows, and whether the player can skip it.
- Intro and exit: what the player controls, and what the scroll and the enemies
  do while they play (T-024).
- Where the sound effects and music come from.

Build:
- Spawn placement: facing (T-028), formation rotation (T-035), and the missing
  Tiled enum values from the 2026-09-25 session.
- Scroll pause, released as decided (T-023).
- Dialog: the box, the map trigger, and the lines.
- Level intro and exit (T-024), and a level complete moment.
- Audio: sound effects for shots, hits, absorbs, pickups and deaths, and one
  music track.
- The level itself: waves from the M2 and M3 rosters, pickups and dialog,
  following the outline.
- A stand-in boss: a large enemy from the roster at a scroll pause. M5 replaces
  it.

Done when: the level plays from start to end with dialog and sound, and it is
the best example of the game.

## M5 Sectioned bosses

Goal: the M4 level ends in a boss fight that is the high point of the level.

Enemy data can list sections today, but that code is untested since the last
design. Do not assume it fits: design bosses from scratch, and keep or replace
that code as the design needs.

Decide first:
- How a boss is authored: in the enemy data, in a new boss format, or in an
  editor.
- Protection: which sections shield others, and how that is shown.
- Phases: what starts the next phase (sections destroyed, health, time), and what
  changes (movement, weapons, sections).
- What destroying a section does: stop its weapon, change the sprite, drop a
  pickup.
- How the boss moves and fires: its own behaviours, or per section.
- Affinity: one for the whole boss, per section, or switched by phase.
- How the fight starts and ends: entry, scroll pause, dialog, death, reward.
- How the player sees boss health: a bar per boss, per section, or none.

Build, each as its own task once the decisions are made:
- Sections that are hit, damaged and destroyed on their own.
- Protection between sections, and how it is shown.
- Phases, and the changes each phase makes.
- Boss movement and firing, as decided.
- Boss health on screen, if decided.
- The boss entry, death and reward, joined to the scroll pause and dialog from
  M4.
- Feel: section destruction, phase changes and the boss death each land hard.
- One boss for the M4 level, in place of the stand-in.

Done when: the boss fight is the best part of the level, and a second boss could
be made from data without new code.

## M6 Run structure

Goal: the game is a run, not a single map.

Today the game starts straight into the stars map, and there are no menus or
saves.

Decide first:
- After the last life: restart the level or the run?
- What carries from one level to the next: weapons and their levels, lives,
  power.
- What a save holds, and when it is made (at level boundaries is the plan).
- One save slot or several?
- What happens between levels: straight on, a short screen, or a story beat.

Build:
- Main menu: new run, continue, quit.
- Pause menu: resume, quit to the main menu.
- Level to level flow, carrying what was decided.
- A second (rough) level to test the flow.
- Save and load at level boundaries, with slots if decided.
- Game over and run complete screens.

Done when: a new run goes through two levels, can be quit and continued, and ends
cleanly.

## M7 Content

Goal: all the levels, in rough form.

Decide first:
- How many levels, and roughly how long a full run takes.
- The story outline: the beats, and which level tells each one.
- Each level's look, its new enemies or weapons, and its bosses, including any
  bosses partway through a level.
- What the M4 level taught about the weapons and enemies, and what to change.

Build:
- Refine the M2 weapons and M3 enemies with what the core taught, and expand
  both in count and variety as the levels need.
- For each remaining level, as its own set of tasks: its look, its waves, its
  bosses, its dialog and its music.
- The full story, as dialog.

Done when: the whole game can be played from start to end.

## M8 Polish

Goal: the game feels finished.

Decide first:
- Score: include it or leave it out.
- Difficulty: one setting, or a choice.
- What the settings menu holds: volume, controls, display.

Build:
- Feel pass across every system, each as its own task: screen shake, hit-stop,
  effects and sound.
- Balance: difficulty curve, weapon levels, power gain, boss health.
- Menu and UI polish, in the same style across every screen.
- A settings menu, as decided.
- Score, if decided.
- Performance: the largest waves stay smooth.

Done when: nothing in the game feels placeholder.
