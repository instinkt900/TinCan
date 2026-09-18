# Review: sprite sheet animation and states

Review of `sprite-animation-states.md` and of the five commits that implement it
(`59449d9`..`08112bb` on `moth-toolkit`), written 2026-09-18.

Everything below comes from reading the code against the toolkit headers in
`moth_toolkit/modules`. Nothing was built or run.

## The plan

Solid and unusually careful. It locates every call site before proposing a
signature, explains *why not* `AnimatorFactory` rather than just picking one,
flags the pivot behaviour change as a visible consequence, and marks two things
as "verify when implementing" (`SetPlaying`, unknown-state handling) instead of
asserting them. Commit granularity is specified up front.

The one thing it does not think through is **state-transition timing**. It
describes `.anim.json` transition clips as supported, and separately says the
helper should guard with `GetCurrentState()`, but never checks that those two
interact. They do not — see finding 1.

## Does the implementation follow the plan?

Yes, closely. All five commits map to the plan's steps 1–5, in order, with the
stated split. Each toolkit API the code calls was checked against its header:

- `Animator::EnterState` already calls `SetPlaying(true)`
  (`anim/src/animator.cpp:93`), so dropping the plan's provisional `SetPlaying`
  call was the correct resolution of that open question.
- `TransitionTo` on an unknown state logs a **warn** and returns
  (`animator.cpp:36`). Called every frame from `SystemMovement` that would spam,
  so the added `m_stateIds` pre-filter is justified, exactly as the plan's
  fallback said.
- `Sprite::GetWidth()/GetHeight()` are the *current frame's* dimensions, so the
  pivot normalisation in `src/system_drawable.cpp:209` is correct. Frames with
  `w/h <= 0` are rejected at sheet parse, so there is no divide-by-zero.
- `DrawImage(image, transform, pivot, flipX)` exists with pivot defaulting to
  `{0.5, 0.5}`, so the static path is pixel-identical to before.
- Flipbook path resolution — `animPath.parent_path() / parsed->flipbook` —
  matches `SpriteSheetFactory`'s own `path.parent_path() / json["image"]`, and
  `../spritesheet_player.png` resolves correctly.
- Authored pivots are all frame centres, so the converted sprites will not
  shift on screen.

One deviation: the plan said to note random animation phase as a follow-up.
That note was never written down anywhere.

## Findings

### 1. Authored transitions will deadlock the animator

`src/system_drawable.cpp:37`

`SetSpriteState` guards with `GetCurrentState() != state`, but
`Animator::TransitionTo` does **not** update `m_currentState` when an authored
transition clip starts — it parks the target in `m_pendingTarget` and leaves
`m_currentState` as the old state (`animator.cpp:47-56`). So while an
`idle → turn_left` transition clip plays, `GetCurrentState()` still returns
`"idle"`, the guard fails, and the next frame calls `TransitionTo("turn_left")`
again. `TransitionTo`'s interrupt policy then cancels the in-flight transition
and re-issues `SetClip`, resetting the clip to step 0. The transition clip never
reaches its end, `OnTransitionCompleted` never fires, and the sprite freezes on
the transition's first frames forever.

Latent today only because `player_ship.anim.json` authors no transitions — the
exact feature the plan sells as the reason to use `.anim.json` over
`sheet` + `clip`. The fix needs the component to remember the *requested* state
rather than comparing against the animator's current one, or an `Animator` API
that exposes the pending target.

### 2. Banking is applied to every moving entity, not the player

`src/system_movement.cpp:142`

The block sits in `UpdatePosition`, which iterates all
`ComponentPosition + ComponentVelocity` entities, so enemies, projectiles and
pickups all get `SetSpriteState` calls each frame. It is harmless right now only
by coincidence: `enemy_wraith`'s synthesised state id happens to equal its clip
name `"idle"`, so the call is a self-transition no-op, and `turn_left` /
`turn_right` are filtered by `m_stateIds`. Name a sheet clip `turn_right` on
some enemy and it starts banking. The plan did say velocity "also covers
non-player entities later", so this is arguably intended, but it deserves a
player/affinity guard or an explicit comment.

### 3. Doc/code mismatch

`src/system_drawable.h:44`

The comment says unknown states "log a warning"; the implementation returns
silently. The silence is the right behaviour (see finding 1's context); the
comment should match.

### 4. Per-entity cost — minor

`m_stateIds` is an `unordered_set<std::string>` copied into every spawned
`ComponentSprite`, duplicating strings already held by the shared `AnimSet` and
by the `Animator`'s own copy. `SetSpriteState` also allocates a
`std::string(state)` per call, per entity, per frame just to do the lookup. For
a wave of drones that is measurable churn in a hot path. A
`HasState(string_view)` on `Animator` would remove both.

### 5. Latent trap — low priority

`src/system_shield.cpp:34` sizes the shield from `sprite->m_image.GetWidth()`.
For animated entries `m_image` is default-constructed, so that returns 0. Safe
today (`player_shield` is still an `image` entry), and it fails silently rather
than crashing, but converting that one sprite to a sheet would collapse the
shield to nothing with no error.

## Verdict

A good solution and a faithful implementation. The data-load validation is
thorough (exactly-one-source check, clip existence for states *and* transitions,
fail-at-load with clear messages), the single-animator code path for both the
`sheet` and `anim` forms is the right call, and the static draw path is
genuinely unchanged.

Fix finding 1 before authoring any transition clip. Findings 2 and 4 are worth
fixing while the code is fresh.
