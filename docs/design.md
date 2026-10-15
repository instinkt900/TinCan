# TinCan Design

A simple top-down shmup. Collect power, carve through large waves of enemies, and
push through a short story across hand-built levels.

This is the guide for what to build. Check new ideas against the
[pillars](#pillars) before they become tasks. Keep each section short and update it
when a decision changes. The order to build it in is the [roadmap](roadmap.md).

## At a glance

| Area                                     | Status      |
| ---------------------------------------- | ----------- |
| [Levels](#levels)                        | Partial     |
| [Enemies](#enemies)                      | Partial     |
| [Bosses](#bosses)                        | Not started |
| [Shield and affinity](#shield-and-affinity) | Partial  |
| [Power weapon](#power-weapon)            | Partial     |
| [Weapons](#weapons)                      | Partial     |
| [Lives and death](#lives-and-death)      | Not started |
| [Story](#story)                          | Not started |
| [Menus and saves](#menus-and-saves)      | Not started |
| [Audio](#audio)                          | Not started |
| [Feel and feedback](#feel-and-feedback)  | Ongoing     |

Status: **Not started**, **Partial** (some of it works), **Done** (matches this
doc), **Ongoing** (never finished; always being tuned).

## Pillars

1. **Power fantasy.** The fun is collecting power and cutting through large
   numbers of enemies.
2. **Progression over score.** The player comes back to get further, not to beat a
   score.
3. **Tactile feel.** Every shot, hit, pickup and death reads clearly and feels
   good.
4. **Simple.** A small set of systems done well. Leave out a feature rather than
   half-build it.

## Core loop

- **In a level:** fly, shoot, switch affinity to absorb bullets, spend power, and
  collect pickups.
- **Across a level:** the ship gets stronger through waves, then faces a boss at
  fixed points.
- **Across the run:** clear levels, follow the story, and save to continue.

## Player

### Shield and affinity

Taken straight from Ikaruga.

- The ship has one of two affinities, light or dark, and switches between them at
  will.
- The shield absorbs enemy bullets of the same affinity, and each absorbed bullet
  adds power.
- Bullets of the other affinity get through the shield.

### Power weapon

- Uses the power the shield absorbed.
- The payoff for playing the affinity system well. It should feel big.

### Weapons

- The ship carries several weapons at once. The first pickup of a weapon adds it.
- Each weapon has several levels, and each later pickup of it raises the level
  (T-039).
- Weapon levels are the main way the ship grows stronger during a run.

### Lives and death

- The player has several lives.
- On death, the ship drops pickups. Collecting them gets the weapons back at the
  levels they were.
- A run ends when the last life is lost. What that sends the player back to is an
  [open decision](#open-decisions).

## Enemies

- Sprite enemies with a range of behaviours and formations.
- Large numbers of weak enemies are the main content. They are there to be carved
  through.
- Enemies fire light and dark bullets that feed the shield.
- Some enemies drop pickups.

## Bosses

- Placed at fixed points in a level, not only at the end.
- Built from sections. Each section can be destroyed on its own, and some are
  protected until others are destroyed.
- Each boss has several phases of behaviour.

## Levels

- Several levels, each built as a Tiled tilemap.
- Enemy waves, bosses and story beats are authored at fixed points in the level.
- Each level has its own look (for example the desert and stars maps).

## Story

- A basic story told through dialog during the levels.
- Dialog must not stop the action for long.

## Menus and saves

- A simple main menu: continue from the last level, or start a new run.
- Possibly several save slots for different runs.

## Audio

- Sound effects and music, both not started.
- Sound is a large part of the [feel](#feel-and-feedback): shots, hits, absorbs,
  pickups and deaths each need a clear sound.

## Feel and feedback

The top priority across every system.

- Hits: flashes, sparks and knockback that show damage landed.
- Absorbs: the player sees and hears the power come in.
- Kills: satisfying explosions, especially in large numbers.
- Pickups and level-ups: each is clearly marked.
- Screen shake and hit-stop, used with care.

## Open decisions

| Question                                                                  | Leaning         |
| ------------------------------------------------------------------------- | --------------- |
| Player health, or one bullet through the shield kills (Ikaruga)?          | Undecided       |
| Score: include it, or leave it out?                                       | Leave out, for now |
| Save slots: one or several?                                               | Maybe several   |
| After the last life: restart the level, or restart the run?               | Undecided       |
| Do passive pickups (fire rate, damage, burst) stay alongside weapon levels? | Undecided     |
| What the power weapon does, and how it is triggered                       | Undecided       |

## Not doing

- Score attack as a focus.
- Randomly generated levels made from themed encounters, from the old notes.
  Levels are authored by hand.
