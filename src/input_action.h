#pragma once

#include <cstddef>

// What the game reacts to. Keys are bound to these in data/input_database.json, so nothing past
// the key event names a key.
enum class InputAction {
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    Fire,
    PowerWeapon,
    SwapAffinity,

    Pause,
    ToggleProfiler,
    ToggleEditor,
    Quit,

    Count, // the number of actions, not an action
};

inline constexpr size_t kInputActionCount = static_cast<size_t>(InputAction::Count);
