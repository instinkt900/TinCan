# TinCan Roadmap

The order in which to build the [spec](spec.md). The spec says what the game is;
this document says only what to build first. Each milestone ends with something
playable. Finish a milestone before starting the next one.

## At a glance

| #  | Milestone                                | Result                                     | Status      |
| -- | ---------------------------------------- | ------------------------------------------ | ----------- |
| M1 | [Core combat](#m1-core-combat)           | One fight feels good and dying matters     | In progress |
| M2 | [Weapon roster](#m2-weapon-roster)       | Five weapons with three levels each        | Not started |
| M3 | [Enemy roster](#m3-enemy-roster)         | Eight enemies, each with its own job       | Not started |
| M4 | [Level 1](#m4-level-1)                   | Outer Dark plays from start to end         | Not started |
| M5 | [Bosses](#m5-bosses)                     | Level 1 ends in the Warden                 | Not started |
| M6 | [Run structure](#m6-run-structure)       | A run from the title screen through levels | Not started |
| M7 | [High scores](#m7-high-scores)           | One global table that every player sees    | Not started |
| M8 | [Content](#m8-content)                   | All four levels, in rough form             | Not started |
| M9 | [Polish](#m9-polish)                     | The full game, finished                    | Not started |

## Rules

- **The spec comes first.** A milestone builds parts of the spec. Anything a
  milestone needs that the spec does not say goes into the spec before it goes
  into a task.
- **Depth before breadth.** One level that plays well comes before four that do
  not. M2 and M3 build enough weapons and enemies to make level 1. M8 refines
  them with what level 1 taught.
- **Feel is part of done.** A feature is not finished until it has what the
  spec's [Feel](spec.md#feel) table asks for, even as a placeholder.
- **Task a milestone when it starts.** Tasks are written for the current
  milestone only, so they do not go stale before they are built.
- **Settle first.** Each milestone lists what the spec leaves open for it. Settle
  those in the spec at the start of the milestone.

## M1 Core combat

Goal: the moment-to-moment fight is fun, and dying matters.

Spec: [Player](spec.md#player), [Shield and affinity](spec.md#shield-and-affinity),
[Power weapon](spec.md#power-weapon), [On-screen display](spec.md#on-screen-display),
and weapon levels from [Weapons](spec.md#weapons).

Today the player has 100 health, and a death ends the game. The shield absorbs
bullets of its own affinity and adds each bullet's damage as power. The power
weapon spends all stored power on one fan of homing shots. Each weapon level is
its own pickup, and passive pickups raise fire rate, damage and burst. Enemies
flash when hit, and the camera can shake. There is no score, no on-screen display
and no sound.

Build:
- Weapon levels through repeated pickups, and 20 power for a pickup at the last
  level (T-039).
- One-hit death: health is removed, and a bullet through the shield or contact
  with an enemy destroys the ship.
- Lives, and the respawn with its safe time (T-022).
- Death drops that give back each weapon at its level.
- A last life lost restarts the level with the state the player entered it
  with, as a game over does. The game over screen comes in M6.
- Score values in enemy data, the score, and an extra life every 100,000 points.
- The power weapon as the spec says: 1 power per absorbed bullet, a cap of 100,
  and 4 missiles for each full segment.
- Passive pickups removed, with their code.
- The on-screen display: lives, score and the power meter.
- Sound effect playback, with placeholder sounds.
- Feel for player death, enemy death, absorb, power release and extra life.
- A test map with a few waves, for replaying the fight.

Done when: the test map is fun to replay, and losing a life hurts but can be
recovered.

## M2 Weapon roster

Goal: the five weapons, each with three levels, each the best choice somewhere.

Spec: [Weapons](spec.md#weapons), [Pickups](spec.md#pickups).

Today there is the main gun and homing missiles. Weapon levels come in M1.

Build:
- Piercing shots and short-range shots, each as its own task.
- The main gun and homing at their spec levels, and spread, laser and side guns
  with all three levels, in data.
- One pickup per weapon, with its icon on a coloured badge.
- Pickups set on a single enemy's death, as well as on a group.
- The collect flash, sound and floating text.
- For each weapon: its shot sprites, hit spark and fire sound.
- A debug tool that gives any weapon at any level.

Done when: each weapon has a situation where it is the best pick, and each
level-up is a clear step up.

## M3 Enemy roster

Goal: the eight enemies, each asking the player for something different.

Spec: [Enemies](spec.md#enemies).

Today there are the danger_1 to danger_6 sprites in light and dark, two enemy
weapons, and asteroids that split on death (T-027).

Settle first: the spec marks enemies as to revisit. Confirm the table before
building it.

Build:
- Each enemy in data, with its health, weapon, behaviours and score.
- Enemy weapons the table needs: the sprayer ring, the heavy's fan and aimed
  shot.
- A carrier that releases fodder while it is alive.
- Scaled sprites per enemy type, for the heavy and carrier.
- Contact rules: chasers and splitters die when they hit the ship.
- Feel for each enemy: hit flash, a death effect sized to it, and its sound.
- A test map with a wave of each enemy.

Done when: each enemy reads at a glance and is tuned against the M2 weapons.

## M4 Level 1

Goal: Outer Dark plays from start to end, and is the best example of the game.

Spec: [Levels](spec.md#levels), [Level 1](spec.md#level-1-outer-dark),
[Dialog](spec.md#dialog), [Audio](spec.md#audio).

Today waves are Tiled point objects in the map, each with an enemy, formation,
behaviour and drop. The map always scrolls, and there is no dialog, intro or
exit.

Settle first: the spec marks levels as to revisit. Confirm the level 1 beats
before placing waves.

Build:
- Spawn placement: facing (T-028), formation rotation (T-035), and the missing
  Tiled enum values.
- The map objects: pickup spawn, scroll pause (T-023), dialog and level end.
- The dialog box, with placeholder lines.
- The level intro and exit, and "LEVEL COMPLETE" (T-024).
- Music playback, with a placeholder level track.
- The level itself, following its beats.
- A stand-in boss: a heavy at a scroll pause. M5 replaces it.

Done when: the level plays from start to end with dialog, sound and music.

## M5 Bosses

Goal: level 1 ends in a boss fight that is the high point of the level.

Spec: [Bosses](spec.md#bosses), the Warden in
[Bosses by level](spec.md#bosses-by-level).

Today enemy data can list sections, but that code is untested since the last
design. Keep it only where it fits the spec.

Settle first: the spec marks bosses as to revisit. It does not yet say how a
section's affinity works, or whether touching a section destroys the ship.

Build:
- Sections that are hit, damaged and destroyed on their own, with wrecked
  sprites.
- Protection, shown darker and with the "tink".
- Phases, and the changes each phase makes to movement and weapons.
- The boss entry and death, joined to the scroll pause, dialog and level exit
  from M4.
- The boss music track.
- The Warden, in place of the stand-in.

Done when: the fight is the best part of the level, and a second boss could be
made from data without new code.

## M6 Run structure

Goal: the game is a run, not a single map.

Spec: [Run structure](spec.md#run-structure),
[Menus and screens](spec.md#menus-and-screens), [Saves](spec.md#saves).

Today the game starts straight into the stars map, and there are no menus,
saves or settings.

Build:
- The screens: title, main menu, pause, game over and credits, without the high
  score table (M7 adds it).
- Level to level flow, carrying lives, weapons, power and score.
- The game over rule, and Restart counting as a game over.
- The save, Continue, and the "Start a new run?" prompt.
- The settings file: volumes, fullscreen and the player name.
- The title music track.
- A rough level 2 on the desert map, to test the flow.

Done when: a new run goes through two levels, can be quit and continued, and ends
cleanly.

## M7 High scores

Goal: every player sees the same high score table.

Spec: [High scores](spec.md#high-scores).

Today there is nothing. The service is a new repository.

Settle first: the HTTPS library (a new toolkit module or a library such as
libcurl), and where the name prompt sits in the screen flow.

Build:
- The service, in its own repository: submit and fetch, signing and the score
  cap.
- The game's HTTPS client.
- Score submission at a game over and at the end of the run.
- The name prompt, kept in the settings file.
- The local queue for scores that cannot be sent.
- The table on the high scores, game over and credits screens, and the title
  screen cycling to it.
- The offline display.

Done when: scores from two machines reach one table, and the game plays the same
with the service down.

## M8 Content

Goal: all four levels, in rough form.

Spec: [Levels](spec.md#levels), [Bosses by level](spec.md#bosses-by-level),
[Story](spec.md#story).

Settle first: the story, and anything level 1 taught about the weapons, enemies,
levels and bosses.

Build:
- Changes to the weapons and enemies that level 1 showed are needed.
- For each of levels 2 to 4, as its own set of tasks: its tileset, waves,
  bosses, dialog and music. Level 3 also has the Picket midboss.
- The story, written as dialog.

Done when: the whole game can be played from start to end.

## M9 Polish

Goal: the game feels finished.

Spec: [Feel](spec.md#feel), [Art](spec.md#art), [Audio](spec.md#audio).

Build:
- The final art and audio, chosen and put in.
- A feel pass against the spec's Feel table, each event as its own task.
- Balance: score values, about one extra life per level, power gain, weapon
  levels and boss health.
- Every screen in the same style.
- Performance: the largest waves stay smooth.

Done when: nothing in the game feels placeholder.
