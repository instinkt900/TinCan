# TinCan Spec

The exact description of the game to build. If it is not in this document, it is
not in the game.

**Status:** draft, 2026-10-16.

- **Agreed:** controls, death and lives, score, shield and affinity, power
  weapon, weapons, pickups, run structure, screens, on-screen display, saves,
  high scores, feel, and where art and audio come from. Score values are agreed
  as starting values to tune.
- **To revisit:** enemies, levels and bosses.
- **Undecided:** the story.
- **Still a first-draft proposal:** anything not listed above.

## How to use this spec

- **The spec decides; the roadmap orders; tasks build.** The
  [roadmap](roadmap.md) sets the order of the work, and each task builds a
  part of this spec. A task that needs something this spec does not say stops
  until the spec says it.
- **Changing the game means changing the spec first.** A new mechanic, weapon,
  enemy, level, menu or screen goes here before it goes in a task. If you would
  rather not write it down, do not build it.
- **Numbers are starting values.** Health, damage, speeds, timings and counts
  live in data and are tuned by playing. Tuning a number does not need a spec
  change. Changing what a thing *does* does.
- **Ideas go in [Parked](#parked).** An idea that is not in the spec is
  written there in one line and left alone.

## The game in one paragraph

TinCan is a vertical-scrolling shmup in four levels, about 25 minutes from start
to end. The player flies a small salvaged ship through an invading fleet to its
flagship. The ship switches between light and dark: its shield absorbs bullets of
its own colour, and its shots do double damage to enemies of the other colour.
Absorbed bullets charge a power weapon that fires a swarm of homing missiles.
Weapon pickups add five weapons and raise their levels, so by the end of a level
the ship cuts through large waves. Each level ends in a sectioned boss. One
bullet through the shield destroys the ship. Kills score points, and points earn
extra lives.

## Pillars

1. **Power fantasy.** The fun is collecting power and cutting through large
   numbers of enemies.
2. **Progression over score.** The player comes back to get further. Score
   rewards killing and earns lives, but it is not the goal.
3. **Tactile feel.** Every shot, hit, pickup and death reads clearly and feels
   good.
4. **Simple.** A small set of systems done well. Leave out a feature rather than
   half-build it.

## Screen and controls

- The play field is the map width (1088 px for a 34-tile map) by 1080 px, and
  the map scrolls down at 60 px/s.
- Keyboard and gamepad both work. Bindings are in `data/input_database.json`
  and can be changed only by editing that file. The game has no screen for
  remapping buttons.

| Action          | Keyboard          | Gamepad          |
| --------------- | ----------------- | ---------------- |
| Move            | Arrow keys        | Left stick, D-pad |
| Fire (hold)     | Space, Ctrl       | A                |
| Switch affinity | X                 | B                |
| Power weapon    | Z                 | X                |
| Pause           | Escape, P         | Start            |

The debug keys (F1 profiler, F2 editors) stay in all builds.

## Player

### Ship

- Moves at up to 800 px/s, with acceleration 4500 and deceleration 3500.
- The hitbox is a 5 px radius at the centre of the ship. The shield (45 px
  radius) is drawn around it and is only for show and absorbing.
- The ship cannot leave the play field.
- Fire is held down; there is no auto-fire toggle.

### Death and lives

- As in Ikaruga, there is no health. **One hit destroys the ship**: one bullet
  that gets through the shield, or one contact with an enemy.
- The run starts with **3 lives**, and the ship holds at most **5**.
- Lives are gained only through [score](#score): one every 100,000 points. A
  life earned at 5 lives is lost.

### Death and respawn

1. The ship explodes (big explosion, screen shake, death sound). The scroll and
   the enemies carry on.
2. Every carried weapon except the main gun drops as its own pickup, spread out
   from where the ship died. Each pickup gives back that weapon at the level it
   had. They drift down with the scroll and are gone after 8 seconds.
3. The main gun keeps its level. Power goes to 0. Score is kept.
4. After 1.5 seconds the ship flies in from the bottom centre of the screen. It is
   safe for 3 seconds and blinks, and it can fire during that time.

The time between death and respawn is the cost of dying: the enemies that get
through while the ship is gone score nothing.

### Score

- Every enemy, and every boss section, has a score value in its data. Killing it
  adds that value, whatever killed it: shots, the power weapon, or the enemy
  ramming the ship. Escaped enemies score nothing.
- A boss also gives a bonus when its core dies.
- Losing a life never removes score. It only costs the points the player misses
  while the ship is gone.
- A game over resets the score to **zero** (see [Run structure](#run-structure)).
  A great score comes from getting through the run without a game over, and the
  best scores from getting through with no deaths.
- Every 100,000 points gives a life (see [Death and lives](#death-and-lives)).
- There is no multiplier or chain bonus.
- Every score value, the boss bonuses and the points per extra life are data
  values, kept where they are easy to find and change. They are expected to move
  a lot as the game is built; the table below is only where they start.

| Enemy    | Points | Enemy    | Points |
| -------- | ------ | -------- | ------ |
| Fodder   | 100    | Chaser   | 200    |
| Gunner   | 300    | Heavy    | 5,000  |
| Aimer    | 300    | Splitter | 200 per piece |
| Sprayer  | 1,000  | Carrier  | 3,000  |

Boss sections give 2,000 each, and a boss's core bonus is 20,000 (50,000 for the
Flagship Heart). A player who kills most of a level should earn about one life
per level.

When the last life is lost, the game over screen shows (see [Run structure](#run-structure)).

## Shield and affinity

- The ship is light or dark, and the switch button changes it at once. There is
  no cooldown. The ship and shield sprites show the current affinity.
- The shield absorbs enemy bullets of the ship's affinity. Each absorbed bullet
  adds **1 power** and plays the absorb effect and sound.
- Enemy bullets of the other affinity pass through the shield and destroy the
  ship if they reach its hitbox.
- Player shots take the ship's affinity at the moment they are fired. A shot
  does **double damage** to an enemy of the other affinity (this works today).
- So the choice is: match the bullets to absorb them, or oppose the enemies to
  kill them faster.

## Power weapon

- Power is stored up to **100**, shown as a meter of 10 segments of 10.
- Pressing the power button spends **every full segment**. Partial segments
  stay. With no full segment, the button does nothing and plays a short "empty"
  sound.
- Each segment fires **4 homing missiles** (50 damage each), so a full meter fires
  40. They fan out from the ship and seek enemies at random.
- The release has a screen flash, a short shake and its own sound. It does not
  clear bullets and does not make the ship safe.

## Weapons

- The ship carries up to five weapons, one of each, and fires them all at once
  while fire is held. A new weapon adds to the ones already carried.
- There is no limit on stacking: a player who collects everything carries all
  five at level 3.
- Each weapon has **3 levels**. The first pickup of a weapon adds it at level 1.
  Each later pickup raises it by one level (T-039).
- A pickup for a weapon already at level 3 gives **20 power** instead.
- The run starts with the main gun at level 1.
- Each level is a full definition in data, not a change to the level before.

| Weapon      | Job                                  | Level 1                                  | Level 2                    | Level 3                              |
| ----------- | ------------------------------------ | ---------------------------------------- | -------------------------- | ------------------------------------ |
| Main gun    | Steady forward damage; the default   | 2 barrels, 5 dmg, every 100 ms           | 4 barrels                  | 6 barrels, 7 dmg                     |
| Homing      | Clears scattered weak enemies        | 2 missiles, 20 dmg, every 500 ms         | 4 missiles                 | 6 missiles, every 400 ms             |
| Spread      | Covers the screen, weak at range     | 5 shots in 40°, 6 dmg, every 150 ms; reach 500 px | 7 shots in 60°    | 9 shots in 80°, 8 dmg                |
| Laser       | Cuts through lines of enemies        | 1 piercing bolt, 10 dmg, every 200 ms    | 2 bolts                    | 3 bolts, 14 dmg                      |
| Side guns   | Covers the sides and behind          | 2 barrels at ±90°, 5 dmg, every 150 ms   | adds 2 rear at ±150°       | adds 2 at ±45°                       |

- A **piercing** shot passes through enemies and damages each one once.
- A **short-range** shot is removed after it has travelled its reach.
- Each weapon has its own shot sprite (light and dark), hit spark and fire sound.

## Pickups

- There is one pickup per weapon, and nothing else (no passive pickups, no lives,
  no power capsules).
- Each pickup shows its weapon's icon on a coloured badge, so it reads before it
  is collected: main gun white, homing orange, spread green, laser blue, side guns
  purple.
- Pickups drift down with the scroll, are collected by touching them (the
  shield's radius counts), and are removed when they leave the bottom of the
  screen.
- Pickups come from:
  - pickup spawns placed in the map,
  - drops set on an enemy spawn or a boss in the level data: a formation's group
    drop (all members killed; this works today) or a single enemy's drop on
    death,
  - the player's own death.
- Which weapon a pickup or drop gives is set in the level data. It is never
  random. Where the pickups go is up to level design.
- On collect: a flash and sound, and the weapon's name and new level float up
  from the ship for 1 second ("SPREAD 2", or "+20 POWER" at max level).

The passive pickups (fire rate, damage, burst) are removed, with their code.

## Enemies

Every type comes in a light and a dark version. An enemy's bullets are always its
own affinity. There is no enemy that switches affinity.

| #  | Enemy    | Art               | Health | Fires                                               | Moves                               | On death        |
| -- | -------- | ----------------- | ------ | --------------------------------------------------- | ----------------------------------- | --------------- |
| 1  | Fodder   | danger_1          | 15     | Nothing                                             | Formations of 6–10 on paths         | Group drop only |
| 2  | Gunner   | danger_2          | 40     | 4-shot burst straight down every 3 s                | Paths, often pausing on screen      | —               |
| 3  | Aimer    | danger_3          | 40     | 1 shot at the player every 2 s                      | Paths                               | —               |
| 4  | Sprayer  | danger_4          | 120    | Ring of 12 slow bullets every 2.5 s                 | Flies in, holds, flies out          | —               |
| 5  | Chaser   | danger_5          | 30     | Nothing; rams                                       | Chases the player                   | —               |
| 6  | Heavy    | danger_6 at 2×    | 600    | 5-way fan every 2 s, and 1 aimed shot every 1 s     | Slow; holds on screen               | Its drop        |
| 7  | Splitter | asteroids         | 80     | Nothing                                             | Drifts; splits 1 → 3 → 2 (T-027)    | Pieces          |
| 8  | Carrier  | danger_6 at 3×    | 400    | Releases 2 fodder every 2 s while alive             | Slow; crosses the screen once       | Its drop        |

- **Bullets** move at 150–400 px/s and are 8 px in radius. Enemy bullets are
  always slow enough to read.
- **Sprayers are the main power source.** Each level places them where the
  player has room to absorb.
- **Contact** with any enemy destroys the ship. Chasers and splitters die too,
  and score; other enemies take no damage from it.
- Score values are in [Score](#score).
- **Hit flash** on every hit; **death effect and sound** on every death.
- Every enemy is removed by its lifetime or by leaving the screen (T-036).
- The art is placeholder until the [art pass](#art).

## Bosses

Bosses are built from **sections**. Each section has its own health, sprite,
weapons and hit flash.

- **Protected** sections take no damage. They are drawn darker, and shots hitting
  them spark off with a "tink" sound. A section is protected until the sections it
  names are destroyed.
- A **destroyed** section explodes, stops firing and changes to a wrecked sprite.
- A boss has **phases**. A phase starts when the sections it names are destroyed.
  A new phase can change the boss's movement and the weapons of its sections.
- The boss dies when its **core** section is destroyed.
- There is no boss health bar. A boss shows damage through hit flashes and
  wrecked sections.
- **Entry:** the scroll pauses, the boss flies in from the top, one or two lines
  of dialog play, then it starts firing. It cannot be damaged during its entry.
- **Death:** 2 seconds of chained explosions and shake, all enemy bullets on
  screen are removed, then its drops (set in the level data) fall and the level's
  exit starts.
- Bosses are authored in the enemy data, as enemies with sections and phases. No
  boss editor.

Every boss is listed under its level below.

## Levels

Four levels, each about 5 minutes. Each one is a Tiled map with its own tileset.
Waves, pickups, scroll pauses and dialog are objects placed in the map.

| Level | Name          | Look             | New this level                                  | Boss                     |
| ----- | ------------- | ---------------- | ----------------------------------------------- | ------------------------ |
| 1     | Outer Dark    | Stars (exists)   | Fodder, gunner, aimer, sprayer, chaser, heavy; main gun, spread, homing | The Warden  |
| 2     | Dust          | Desert (exists)  | Splitter, carrier; laser                        | The Sandworm Rig         |
| 3     | The Fleet     | Fleet tileset    | Side guns; dense mixed waves                    | Midboss Picket, then the Dreadnought |
| 4     | The Flagship  | Flagship hull    | Nothing new; everything at once                 | The Flagship Heart       |

### Map objects

| Object        | What it does                                                               |
| ------------- | -------------------------------------------------------------------------- |
| Enemy spawn   | A wave, as today: enemy, formation, behaviour, count, drop, facing (T-028)  |
| Pickup spawn  | One pickup at a point                                                      |
| Scroll pause  | Stops the scroll until every enemy spawned since the last pause is gone, or a boss dies (T-023) |
| Dialog        | Shows a list of lines when the scroll reaches it                           |
| Level end     | Starts the exit                                                            |

While the scroll is paused, the background still moves (the parallax stars), so
the screen never looks frozen. Spawns below the pause wait for it to release.

### Level intro and exit

- **Intro:** the ship flies in from below the screen to its start point over 1.5
  seconds while the level name shows. The player gets control when it arrives.
- **Exit:** after a boss dies, the player has 3 seconds to collect the drops,
  then loses control and the ship flies off the top of the screen.
  "LEVEL COMPLETE" shows for 3 seconds, then the next level starts (and the game
  saves).

### Level 1: Outer Dark

Times are scroll time from the start of the level.

| Time      | Beat      | What happens                                                                 |
| --------- | --------- | ---------------------------------------------------------------------------- |
| 0:00      | Intro     | Fly-in; dialog: launch                                                       |
| 0:05–0:50 | Opening   | Light fodder formations, then dark; first gunners. Main gun pickup at 0:30   |
| 0:50–1:40 | Absorb    | One light sprayer alone, with dialog about the shield; then a dark one; then aimers. Spread pickup |
| 1:40–2:30 | Build-up  | Mixed fodder and gunners from both sides; chasers. Homing pickup             |
| 2:30      | Set piece | Scroll pause: a heavy with fodder escorts. It drops a main gun pickup        |
| 2:30–4:00 | Push      | Dense mixed waves; sprayers to fill power for the boss                       |
| 4:00      | Boss      | Scroll pause, dialog, the Warden                                             |
| —         | Exit      | Fly-out, level complete                                                      |

By the boss, a player who collected everything has main gun 3, spread 1 and
homing 1.

### Level 2: Dust

Same shape as level 1. Splitters in the opening, a carrier as the set piece, the
laser pickup in the build-up. Ends with main gun 3 and spread, homing and laser
at 2 for a player who collected everything.

### Level 3: The Fleet

Two halves. The first half ends in the **Picket** midboss at 2:00, and the side
guns pickup drops from it. The second half is the densest waves in the game,
ending with the Dreadnought. A player who collected everything ends with every
weapon at 3.

### Level 4: The Flagship

The ship flies along the flagship's hull. Short, hard waves of every enemy type,
two heavies as set pieces, and no new pickups apart from drops. Ends with the
Flagship Heart. Clearing it ends the run.

### Bosses by level

| Boss                | Sections                                                                 | Phases                                                                                     |
| ------------------- | ------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------ |
| The Warden          | 2 turrets (300 each, one light, one dark); core (1500), protected until both turrets are gone | 1: strafes across the top third; turrets fire aimed 3-shot bursts. 2: core opens, fires alternating light and dark rings, strafes faster |
| The Sandworm Rig    | 4 drills (250 each); core (2000), protected until the drills are gone    | 1: drills fire fans as it sweeps side to side. 2: core fires spirals and releases chasers  |
| Picket (midboss)    | 1 core (1200)                                                            | 1: fires sprayer rings in both colours. Leaves after 40 s if not killed (no drop)         |
| The Dreadnought     | 2 wing guns (400 each); 2 engines (500 each); core (2500), protected until the engines are gone | 1: wing guns fire aimed bursts. 2 (wing guns gone): engines fire rings. 3 (engines gone): core fires everything and moves |
| The Flagship Heart  | 4 shield nodes (400 each, 2 light, 2 dark); core (4000), protected until the nodes are gone | 1: nodes fire rings of their colour. 2: core switches its bullets' colour every 4 s. 3 (below half health): faster, adds aimed shots |

## Story

**Undecided.** Everything in this section, including the dialog rules, is a
placeholder from the first draft. Nothing here is built until the story is
decided.

Told only through dialog, in short radio lines between two characters:

- **The Pilot**, flying the Tin Can, a patched-up salvage ship.
- **Rook**, the mechanic who built it, on the radio from a hidden base.

The outline:

1. **Outer Dark.** An invading fleet arrives. Every real warship is gone; the Tin
   Can is what is left. Rook talks the Pilot through the shield.
2. **Dust.** The Pilot falls back to a desert world to find more weapons in the
   wrecks. The fleet follows.
3. **The Fleet.** The Pilot turns and goes straight at the fleet.
4. **The Flagship.** The Pilot reaches the flagship and destroys its heart. The
   fleet breaks up.

About 6 to 10 lines per level: two at the intro, two to four during the level,
two at the boss, one at the exit.

### Dialog rules

- A box across the bottom of the screen with a portrait, a name and one line.
- Each line shows for 3 seconds, or longer for a long line (about 15 characters
  per second). The action carries on behind it.
- It cannot be skipped and never pauses the game.
- Lines are in a data file, keyed by id, and map dialog objects name the ids.

## Run structure

- A run is levels 1 to 4 in order.
- Carried from one level to the next: lives, weapons and their levels, power,
  score, and progress towards the next extra life.
- **Game over** (last life lost): the level restarts with **3 lives** and the
  weapons, levels and power the player had when they *entered* that level. The
  score goes back to **zero**, and so does the count towards the next extra
  life. The player can replay a level to learn it, but a score after a game over
  starts again from nothing.
- The run is complete when the Flagship Heart dies.

## Menus and screens

A very basic flow. All screens are moth::ui layouts, in one style.

```
Title ⇄ High scores ──any button──▶ Main menu ──New / Continue──▶ Level
                                       ▲  │ High scores ──back──┘
                                       │  └ Exit
Level ──Pause──▶ Pause menu ──Resume──▶ Level
                    ├ Restart ──▶ Level (counts as a game over)
                    └ Quit ──▶ Main menu
Level ──last life lost──▶ Game over ──Retry──▶ Level
                             └ Quit ──▶ Main menu
Last level cleared ──▶ Credits ──▶ Main menu
```

| Screen      | Contents                                                                  |
| ----------- | ------------------------------------------------------------------------- |
| Title       | The game name and "Press any button". After 10 s it changes to the high scores screen, and back after another 10 s, until a button is pressed |
| Main menu   | Continue (only with a save), New, High Scores, Exit                       |
| High scores | The [high score table](#the-screen); any button goes back                 |
| Pause       | Resume, Restart, Quit (to the main menu)                                  |
| Game over   | "GAME OVER" and the high score table with this run's score; Retry, Quit   |
| Credits     | After the last level: the high score table with this run's score, then the credits; then the main menu |

- **New** with a save present asks "Start a new run? Your saved run will be
  lost."
- Escape (or Start) opens the pause menu. It no longer quits the game.
- **Restart** on the pause menu counts as a game over: the score goes to zero.
  Otherwise a player could restart before losing their last life and keep their
  score. The score is not submitted.
- "LEVEL COMPLETE" is not a screen: it shows over the game during the level's
  [exit](#level-intro-and-exit).
- There is **no settings screen**. Music volume, sound volume, fullscreen and the
  player name are in a settings file that can be edited by hand.

### On-screen display

Only the basics:

| Where        | What                                                   |
| ------------ | ------------------------------------------------------ |
| Top left     | Lives as ship icons                                    |
| Top right    | Score                                                  |
| Bottom left  | Power meter, 10 segments; full segments glow           |

Nothing else. The ship and shield show the affinity, the ship's shots show its
weapons, and the floating pickup text shows a new level.

## Saves

- **One save slot.**
- The game saves at the start of each level: the level number, lives, weapons and
  their levels, power and score.
- Continue loads it and starts that level.
- A game over sets the saved score to zero at once, so quitting and continuing
  does not get the old score back.
- Completing the run deletes the save.
- The settings file is separate and is kept.

## High scores

Global high scores, kept by a small central service, so every player sees the
same table.

### What a score is

- A score is submitted at a **game over** (before the score goes back to zero)
  and when the last level is cleared. Nothing else submits a score: not a lost
  life that leaves lives in hand, not Restart, and not Quit. A score of zero is
  not submitted.
- Each entry holds: the player name, the score, the furthest level reached, whether
  the run was completed, the number of lives lost, the date and the game version.
- One table for the whole game. There are no tables per level or per version.

### Player name

- 3 to 12 letters, digits and spaces.
- The first time a score would be submitted with no name set, the game asks for
  one and keeps it in the settings file. Leaving it empty sends the score as
  "PILOT".
- With no settings screen, the name is changed afterwards only by editing the
  settings file.
- There are no accounts or passwords. The same name from two machines is two
  different players who happen to share a name.

### The screen

The table has 10 rows, on one screen with no pages:

- **Rows 1–9:** the global top 9 scores: rank, name, score, furthest level (with
  a mark for a completed run) and date. Every score counts, so one name can
  appear more than once.
- **Row 10:** the local player's row, highlighted, with its global rank.
  - On the **high scores screen**, it is the local player's best score.
  - On the **game over** and **credits** screens, it is the score for the run
    that just ended.
- When the local player's score is itself in the top 9, it is highlighted there
  and row 10 is left out.
- Before row 10 has a rank from the service (still sending, or offline), it shows
  the score with no rank.

### Offline

- The game works fully without the service. It never blocks play waiting for it.
- A score that cannot be sent is kept in a local queue and sent the next time the
  service can be reached.
- With no connection, the high scores screen shows the last table it fetched,
  marked "offline", or "No connection" if it never fetched one.

### The service

- A small web service, self-hosted on the developer's VPS or homelab, in its own
  repository.
- HTTPS with JSON. Two calls: submit a score (returns its rank), and fetch the
  top 9 (with the player's best and its rank).
- The game talks HTTPS through a new toolkit module or a library such as
  libcurl; which one is chosen when the work starts.
- Each submission is signed with a key built into the game, and the service
  rejects unsigned scores and scores above the most the game could give. This
  stops casual tampering, not a determined cheat. The developer can delete
  entries by hand.
- The service keeps every submitted score, and the table ranks every score, not
  only each name's best.

## Audio

- **Sound effects** for: player shot (per weapon), enemy shot, hit, absorb,
  player death, enemy death (small and large), pickup, level-up, extra life,
  power release, power empty, protected-section tink, section destroyed, boss
  death, dialog line, menu move and select.
- **Music:** one looping track per level, a boss track, and a title track. Six
  tracks in all.
- Audio follows the same rule as [art](#art): placeholder sounds and music from
  anywhere, free or paid, until the final step. The final audio may be
  commissioned or a paid pack.
- Music and sound volumes are in the settings file.

## Feel

What each event must have before it counts as done. A placeholder effect counts
until the art pass.

| Event              | Must have                                                        |
| ------------------ | ---------------------------------------------------------------- |
| Player fires       | Muzzle flash, shot sound                                         |
| Shot hits enemy    | Enemy flashes white, spark at the hit point                      |
| Absorb             | Bullet pulls into the shield, shield pulses, absorb sound, meter ticks |
| Enemy dies         | Explosion sized to the enemy, death sound; holds up with 50 on screen |
| Large enemy dies   | Bigger explosion, shake, 50 ms hit-stop                          |
| Pickup             | Flash, sound, floating "WEAPON N" text                           |
| Power release      | Screen flash, shake, sound, missiles fan out                     |
| Player dies        | Big explosion, strong shake, 100 ms hit-stop, death sound        |
| Extra life         | "1UP" floats up from the ship, sound, the new icon flashes       |
| Boss section dies  | Explosion, shake, wrecked sprite                                 |
| Boss dies          | 2 s of chained explosions, the strongest shake, bullets cleared  |

Shake and hit-stop are used only where the table says.

## Art

- For most of the project, the art is cobbled together from free and paid
  assets, starting with what is there today (danger sprites, asteroids,
  explosions, stars and desert tilesets).
- The final art is commissioned or a paid asset pack, chosen at the final step,
  in the polish milestone (M8).
- The final art covers: the player ship and shield in both affinities, eight
  enemy types in both affinities, five bosses with wrecked sections, five pickup
  badges, shot sprites per weapon, explosions in three sizes, four tilesets, two
  portraits and the screens.

## Not in the game

| Feature                                | Why                                        |
| -------------------------------------- | ------------------------------------------ |
| Accounts, friends lists, replays       | A name and a global table are enough       |
| Strong anti-cheat                      | Signing and a score cap only               |
| Score multipliers, chain bonuses       | Simple                                     |
| Difficulty settings                    | One difficulty, tuned well                 |
| Passive pickups (fire rate, damage)    | Weapon levels replace them                 |
| Bombs that clear bullets               | The shield is the defence                  |
| Focus or slow movement                 | Simple                                     |
| Enemies that switch affinity           | Only the final boss does it                |
| Mid-level checkpoints, continues       | A lost run restarts the level              |
| Several save slots                     | One run at a time                          |
| Control rebinding screen               | The input data file covers it              |
| Settings screen                        | A settings file covers it                  |
| Boss health bar, weapon display        | The on-screen display shows only the basics |
| Shops or upgrades between levels       | Pickups are the only progression           |
| Randomly generated levels              | Levels are authored by hand                |
| Multiplayer                            | Simple                                     |
| Generic data editor (`data-editor.md`) | Not needed to ship; parked                 |

## What changes from today

The gap between the game now and this spec. The roadmap turns these into
milestones.

- **Player:** health 100 goes; one hit destroys the ship. Lives, respawn and
  death drops are new.
- **Score:** score values per enemy and boss, the score display and extra lives
  are new.
- **Power:** an absorbed bullet gives 1 power, not its damage; the cap of 100,
  the segments and 4 missiles per segment are new.
- **Weapons:** levels through repeated pickups (T-039); spread, laser and side
  guns are new; piercing and short-range shots are new.
- **Pickups:** the passive pickups and their code are removed; badge art and
  floating text are new.
- **Enemies:** stats as in the table; sprayer rings, carrier release while alive
  and scaled sprites per enemy type are new.
- **Bosses:** sections, protection and phases are built from scratch (the existing section code is kept only if it fits).
- **Levels:** scroll pause (T-023), dialog, pickup spawns, level end, intro and
  exit (T-024), facing (T-028) and formation rotation (T-035).
- **High scores:** the high scores screen, the player name, the local queue and
  the high score service (a new repository) are new.
- **Everything else is new:** menus, saves, audio, the on-screen display, two
  more maps and the story.

## Parked

Ideas that are not in the game. One line each. Moving one into the spec is a
decision, not a task.

- Turrets fixed to the map.
- Mines that burst into bullets.
- A repeat count on looping behaviours.
- Rotation per behaviour step, turning a formation as it moves.
