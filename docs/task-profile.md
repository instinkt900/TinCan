# Task profile

What `/task-session` needs to know about TinCan. The process is in the shared
workflow, `/home/mcotton/Development/moth/.claude/tasks/workflow.md`.

## Setup

`conan install . --build=missing -s build_type=Debug` creates `build/Debug`. The
session checks that `build/Debug` exists.

## Build

- Incremental: `cmake --build --preset conan-debug`
- Clean: `cmake --build --preset conan-debug --clean-first`

The Debug build uses `-Wall -Werror` on Linux (`/W3 /WX` on MSVC). It does **not**
run clang-tidy: `CMakeLists.txt` sets no `CXX_CLANG_TIDY`, so the `.clang-tidy`
file serves clangd in the editor only. A session still writes code that satisfies
it, but the build will not catch a finding.

## Tests

none. The project has no automated tests, and no test framework is linked.

## Launch check

`tools/smoke_launch.sh [seconds, default 5]`.

It runs `build/Debug/tincan` in a temporary directory with `assets/`, `data/` and
`tiled_project/` linked in, waits for the window titled `TinCan`, lets it play for
a few seconds, then closes it with `WM_DELETE_WINDOW` so normal shutdown runs.

- Pass: prints `SMOKE LAUNCH PASSED`, exits 0. The app exited with code 0 and the
  log holds no `[warning]` or `[error]` record.
- Fail: prints `SMOKE LAUNCH FAILED`, the reasons and the kept log path, exits 1.
- Cannot run: prints `SMOKE LAUNCH CANNOT RUN`, exits 2.

Debug enables the Vulkan validation layers, so this is also what catches renderer
regressions. Two shutdown validation errors are allowed by name,
`VUID-vkDestroyBuffer-buffer-00922` and `VUID-vkDestroySampler-sampler-01082`.
They are a destruction-order problem in gfx (moth_toolkit T-002), and the
allowance is removed once TinCan picks up the fixed `moth_graphics`. Any other
validation error still fails the check.

It needs an X11 display, `xdotool` and `python3`.

TinCan is a windowed Vulkan game with no headless mode, so the check tells a
session that the game starts, runs and shuts down cleanly, and nothing more.
Every requirement that can be judged only by looking at the game goes in the
task's **Manual verification** field for the user to check by hand.

A session must not launch the app any other way. The app reads and writes
`imgui.ini` in the current directory, and it needs `assets/`, `data/` and
`tiled_project/` beside it.

## Rules

- **New `.cpp` files are added to `SOURCES` in `CMakeLists.txt`.** The list is
  written out by hand; there is no glob.
- **Game data lives in `data/`, not in code.** Enemies, behaviours, sprites,
  weapons, projectiles, pickups and spawners are named entries loaded through
  `GameData`, referenced by name with `DataRef`. A session does not hard-code a
  value that belongs in a database entry.
- **Debug and editor UI is toggled, never left on.** The behaviour editor is
  behind F2 (`GameLayer::OnKeyEvent`) and the profiler behind F1. A session does
  not leave an editor panel drawing by default, and does not commit commented-out
  draw calls as a way of disabling one.
- **Systems keep their shape.** A system is a `SystemX::Update(GameWorld&,
  uint32_t)` free-standing class over components, registered in
  `GameWorld::Update`. New per-entity state goes in a component, not in a static.
- The workspace `CLAUDE.md` applies: C++17, surgical changes, no cleanup
  smuggled into a feature, and `-Wall -Werror` kept clean.

## Compatibility

`data/behaviour_database.json` is disposable test data. T-002 rewrites it by hand
and a session may change its schema freely.

Everything else must still load:

- the Tiled maps in `tiled_project/maps/` and the class definitions in
  `tiled_project/tincan.tiled-project`,
- `data/enemy_database.json`, `data/sprite_database.json`,
  `data/sprite_database_enemy.json`, `data/weapon_database_player.json`,
  `data/weapon_database_enemy.json`, `data/projectile_database.json`,
  `data/pickup_database.json` and `data/spawner_database.json`.

A task that needs one of those to change is blocked, not a judgement call.
