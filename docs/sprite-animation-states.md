# Sprite sheet animation and states for sprite data

Plan for teaching TinCan's sprite database about `moth::gfx` sprite sheets and
`moth::anim` animation states, so a sprite entry such as `player_ship` can name
states (`idle`, `turn_left`, `exploding`) that each play a clip from a sheet.

Target branch: `moth-toolkit`.

## Goal

A sprite entry in `data/sprite_database.json` can be described in three ways:

1. **A single image** — what every entry uses today. Unchanged.
2. **A sprite sheet plus one clip** — a looping animation with no states.
3. **An animation set** — named states, each playing a clip, with the optional
   transition clips and `onComplete` chaining that `moth::anim` provides.

Game code asks for a state by name (`"turn_left"`), never for a clip or a frame.

## Where things stand

- `SpriteData` (`src/gamedata_sprite.h:23`) holds a `gfx::Image`, plus `scale`,
  `blend_mode`, `color`, `rotation`, `z_order` and an optional `sheet_data`.
- `sheet_data` (`SpriteSheetData`, `src/gamedata_sprite.h:11`) describes a
  uniform grid (`cell_dimensions`, `grid_dimensions`, `fps`). No entry in
  `data/` uses it, so removing it costs nothing.
- `ComponentSprite` (`src/system_drawable.h:31`) copies those fields, expands
  `sheet_data` into `m_cellRects` in its constructor
  (`src/system_drawable.cpp:13`), and keeps `m_cellIndex`, `m_fps`,
  `m_animationTime` and `m_flashTime`.
- `SystemAnimation::Update` steps `m_cellIndex` through every cell at `m_fps`,
  looping forever. There is no way to select a frame or a subrange.
- `SystemDrawable::Draw` builds a `DrawImage` list, sorts it by `m_zOrder`, then
  for each one sets blend mode and colour (with the hit flash mixed in) and calls
  `graphics.DrawImage(frame, transform)` with a `Transform2D` carrying position,
  angle and scale.
- Explosions are ordinary sprite entities: a `sprites` entry names the sheet and
  clip, and `SystemDrawable::CreateOneShotSprite` spawns it with an `OneShotTag`
  that `SystemAnimation` turns into a `DeadTag` once the clip stops. What an
  entity spawns when it dies comes from its `death_sprite` entry in the data.
- `ComponentSprite` is emplaced from a `SpriteData const&` (plus an optional
  `Affinity`) at seven sites: `src/game_world.cpp:128` and `:160`,
  `src/system_projectile.cpp:133`, `src/system_enemy_spawner.cpp:35` and `:56`,
  `src/system_pickup.cpp:34`.

## The data layers

Three files, each referencing the one below it.

### 1. The sprite sheet descriptor (from moth_sprite / moth_packer)

`assets/sprites/player_ship.json` — the format `SpriteSheetFactory` loads and
`assets/sprites/explosion.json` already uses:

```json
{
  "image": "player_ship.png",
  "frames": [
    { "x": 0,  "y": 0, "w": 48, "h": 48, "pivot_x": 24, "pivot_y": 24 },
    { "x": 48, "y": 0, "w": 48, "h": 48, "pivot_x": 24, "pivot_y": 24 }
  ],
  "clips": [
    { "name": "idle", "loop": "loop",
      "frames": [ { "frame": 0, "duration_ms": 100 },
                  { "frame": 1, "duration_ms": 100 } ] },
    { "name": "bank_left", "loop": "loop",
      "frames": [ { "frame": 2, "duration_ms": 80 } ] }
  ]
}
```

Owns frames, pivots, per-step durations and loop type (`Stop`, `Reset`, `Loop`;
see `SpriteSheet::LoopType`). Knows nothing about the game.

moth_sprite writes a plain `.json` extension. The `.flipbook.json` name in the
`SpriteSheetFactory` and moth_anim docs is only a suggestion, not a requirement,
so this plan uses `.json` throughout.

### 2. The animation set (`.anim.json`, consumed by moth::anim)

`assets/sprites/player_ship.anim.json`:

```json
{
  "initial": "idle",
  "flipbook": "player_ship.json",
  "states": [
    { "id": "idle",       "clip": "idle" },
    { "id": "turn_left",  "clip": "bank_left" },
    { "id": "turn_right", "clip": "bank_right" },
    { "id": "hit",        "clip": "hit_flash", "onComplete": "idle" }
  ],
  "transitions": [
    { "id": "idle.turn_left", "from": "idle", "to": "turn_left",
      "clip": "bank_left_start" }
  ]
}
```

Owns the state names the game uses and the clip each one plays. `flipbook`
resolves relative to this file. Field meanings are in
`moth_toolkit/modules/anim/README.md`; the structures are `AnimSet`,
`StateSpec` and `TransitionSpec` in `<moth/anim/anim_set.h>`.

### 3. The sprite database entry

`data/sprite_database.json` keeps the drawing settings and the in-game name that
enemy, projectile and pickup data refer to:

```json
"player_ship": {
    "anim": "assets/sprites/player_ship.anim.json",
    "blend_mode": "Alpha",
    "z_order": 1
},
"enemy_drone": {
    "sheet": "assets/sprites/enemy_drone.json",
    "clip": "idle",
    "blend_mode": "Alpha"
},
"player_shield": {
    "image": "assets/shield.png",
    "blend_mode": "Alpha",
    "z_order": 5
}
```

Reference chain:

```
enemy_database ──"sprite": "player_ship"──► sprite_database.json
sprite_database.json ──"anim"──►            player_ship.anim.json
player_ship.anim.json ──"flipbook"──►       player_ship.json
player_ship.json ──"image"──►               player_ship.png
```

## Schema changes to `SpriteData`

Exactly one of `image`, `sheet` or `anim` must be present; more than one is an
error. `clip` is only valid with `sheet`, and defaults to the sheet's first clip
when omitted. `scale`, `blend_mode`, `color`, `rotation` and `z_order` keep their
current meaning in all three cases.

| Key | Type | Notes |
|---|---|---|
| `image` | path | As today. Mutually exclusive with `sheet`/`anim`. |
| `sheet` | path | A sprite sheet descriptor. |
| `clip` | string | With `sheet` only. Defaults to the first clip. |
| `anim` | path | An `.anim.json`. |

`sheet_data` and `SpriteSheetData` are removed along with the grid code in
`ComponentSprite`'s constructor. Nothing in `data/` uses them. Keep the removal
in its own commit, before the feature work.

Load failures behave like the current image path: log through `spdlog::error` and
throw, so bad data fails at load rather than at draw.

## Code changes

### `src/gamedata_sprite.h`

`from_json` picks a branch on which key is present. `SpriteData` gains, beside
the existing `image`:

- `std::shared_ptr<moth::gfx::SpriteSheet> sheet` — the loaded sheet, shared by
  every entity using this entry.
- `std::string clip` — empty unless the `sheet` form was used.
- `std::optional<moth::anim::AnimSet> animSet` — the parsed set for the `anim`
  form.

Both non-image forms need the sheet loaded at data-load time, while
`GameData::s_currentContext` is valid (it is only set during
`GameData::Load`, see `src/gamedata.cpp:59`). Use
`s_currentContext->surfaceContext.GetAssetContext().GetSpriteSheetFactory()
.GetSpriteSheet(path)`, which caches by path, so several entries sharing a sheet
load it once.

For the `anim` form, read the file's text, call
`moth::anim::ParseAnimSet(text)`, then resolve `set.flipbook` relative to the
`.anim.json`'s own directory and load that sheet. Store both in `SpriteData`.

**Why not `AnimatorFactory`:** it would do this loading and caching, plus
clip-existence and loop-type validation, but it only exposes
`CreateAnimator(path)` — it never hands back the set or the sheet, and it needs
an `AssetContext` that outlives loading. Using it would mean giving
`ComponentSprite`'s constructor access to a factory owned by `GameWorld`,
changing the constructor's signature and all seven emplace sites. Loading
directly keeps the signature as it is. The cost is losing the factory's
validation, so add the check that matters most: every `StateSpec::clip` (and
each `TransitionSpec::clip`) resolves via `sheet->GetClipDesc()`, and the
`sheet` form's `clip` exists. Fail at load if not.

If a later feature needs `AnimatorFactory`'s caching of sets, switch to option B
above; it is a mechanical change.

### `ComponentSprite` (`src/system_drawable.h`, `.cpp`)

Replace `m_image` + `m_cellRects` + `m_cellIndex` + `m_fps` +
`m_animationTime` with one of two representations, kept in the same component so
`SystemDrawable`'s single view and z-sort still work:

- `moth::gfx::Image m_image` for the static case (as today).
- `std::shared_ptr<moth::anim::Animator> m_animator` for both animated cases.

The `sheet` form has no states of its own, so build a one-state `AnimSet` in the
constructor (`initial` and the state id can both be the clip name) and construct
`Animator(sheet, set)` with it. That keeps one animated code path instead of
two, and means a `sheet` entry can gain states later by moving to `anim`.

`Animator`'s constructor enters the initial state, but `gfx::Sprite` starts
paused — `SetClip` leaves the playing state alone, and `Sprite::SetPlaying(true)`
is what starts it (see `SetPlaying`'s docs in
`<moth/graphics/graphics/sprite.h>`). Call
`m_animator->GetSprite().SetPlaying(true)` in the constructor. Verify this
against `Animator`'s behaviour when implementing; if the animator already starts
playback, drop the call.

Keep `m_scale`, `m_blendMode`, `m_color`, `m_rotation`, `m_zOrder`, `m_flashTime`
and the `Affinity` tinting as they are.

Entities animating in lockstep is a real risk for sheets shared across many
spawns (a wave of identical drones all on frame 0). `Animator` gives each entity
its own playback state, so a random start is possible but needs a deliberate
call — there is no "start at a random frame" API. Leave it out of the first
version and note it as a follow-up.

### `SystemAnimation` (`src/system_animation.cpp`)

Replace the cell-stepping loop with `sprite.m_animator->Update(ticks)` for
entities that have an animator. Note the unit change: the animator and
`gfx::Sprite` take milliseconds (`uint32_t ticks`), while the old code converted
ticks to seconds for `m_fps`. Pass `ticks` straight through.

### `SystemDrawable::Draw` (`src/system_drawable.cpp`)

The existing draw path must stay, because it applies rotation, scale, z-order,
blend mode, the affinity tint and the hit flash. The `gfx::DrawSprite` helpers
take neither rotation nor a colour, so they are not usable here. Instead, take
the frame from the animator's sprite and keep drawing through
`IGraphics::DrawImage`:

- `DrawImage` has an overload taking `(image, transform, pivot, flipX, flipY)`,
  with `pivot` normalized (`{0.5, 0.5}` is the centre) — the point of the image
  that lands on `transform.position`.
- For animated sprites: source rect from `Sprite::GetCurrentFrameRect()`, pivot
  from `Sprite::GetCurrentFramePivot()` (pixels from the frame's top-left)
  divided by the frame's width and height, and `Sprite::GetFlipX()`.
- For static sprites: keep passing the whole image with the default centre
  pivot, so nothing about existing entries changes on screen.

`DrawImage`'s pivot differs from the current behaviour, which always draws
centred. A sheet whose pivots are not frame centres will sit differently from the
same art as a plain image. That is the correct behaviour (it is what pivots are
for), but expect to adjust positions when converting an existing sprite.

`DrawImage`'s `Image` for a frame is built the way the current code does it:
`moth::gfx::Image const frame(sprite.GetImage().GetTexture(), sourceRect)`.

### State changes from game code

Add a small helper next to `ComponentSprite`, for example
`SetSpriteState(ComponentSprite&, std::string_view state)`, that calls
`TransitionTo` only when `GetCurrentState()` differs — re-entering a state
restarts its clip and re-fires callbacks. It should do nothing for a static
sprite, so callers need no branch.

First user: the player ship's banking. `SystemMovement` already reads
`ComponentInput` for `Key::Left` and `Key::Right`
(`src/system_movement.cpp:126-137`). Drive the state from the same place, or from
the resulting `ComponentVelocity.x` — velocity is the better choice, because it
also covers non-player entities later. Map to `turn_left`, `turn_right` or
`idle`.

A state a sprite does not define is a no-op that logs, so it is safe for
`SystemMovement` to ask for `turn_left` on entities whose sprite has only
`idle`. Confirm `Animator::TransitionTo`'s handling of an unknown state id when
implementing; if it is noisier than that, have the helper check the set first.

### Explosions

Explosions are one-shot sprite entities spawned from the dying entity's
`death_sprite`, so they go through the sprite database like anything else. An
`exploding` state on the ship itself is still out of scope: it would need the
entity to outlive its own death so the clip can finish, which changes entity
lifetime handling. Worth doing later, on its own.

### Build files

- `conanfile.py`: add `self.requires("moth_anim/0.1.0")`.
- `CMakeLists.txt`: add `moth::anim` to `target_link_libraries` (about line 45).
- `moth_anim` depends on `moth_core` and `moth_graphics`, both of which TinCan
  already gets through `moth_bridge`.
- The `moth_anim/0.1.0` recipe is on the Artifactory remote (checked
  2026-09-18), so no local package creation is needed.

## Suggested order of work

1. Remove `sheet_data`/`SpriteSheetData` and the grid code. No data changes.
2. Add `moth_anim` to the Conan and CMake files.
3. Add the `sheet` + `clip` form end to end: loading, `ComponentSprite` with a
   synthesised one-state set, `SystemAnimation`, `SystemDrawable`. Convert one
   enemy to a sheet to prove it.
4. Add the `anim` form: `ParseAnimSet`, flipbook path resolution, clip
   validation.
5. Author `player_ship.anim.json` with `idle`, `turn_left` and `turn_right`, and
   drive it from the player's horizontal velocity.
6. Update `NOTES.md` if it mentions sprite data.

Steps 3 and 4 are each a commit; so are 1, 2 and 5.

## Open decisions

- **Is the `sheet` + `clip` form worth keeping?** It saves authoring a
  two-line `.anim.json` for every animated sprite that has no states. Since it
  becomes a synthesised one-state set internally, it costs only the extra
  branch in `from_json`. Drop it if you would rather have one way to do things.
- **Random animation phase** for crowds of identical enemies (see above).
- **Where state decisions live.** This plan puts the player's mapping in
  `SystemMovement`. If more entities gain states, a dedicated system that maps
  entity condition to sprite state is probably cleaner than spreading
  `SetSpriteState` calls across systems.

## Verification

No automated tests cover this; TinCan has none. Verify by running the game:

- The unchanged sprites (shield, projectiles, pickups, existing enemies) draw
  exactly as before — position, tint, blend and z-order.
- The converted enemy plays its clip and loops.
- The player ship switches to the banking states when moving sideways and
  returns to `idle`, with no clip restarting every frame.
- Malformed data fails at load with a clear message: a missing file, a clip name
  that no clip matches, and both `image` and `anim` on one entry.
