#pragma once

#include "input_action.h"
#include <moth/ecs/world.h>
#include <moth/ui/events/event_key.h>
#include <array>

struct InputData;
class GameWorld;

// What an entity is being asked to do this frame, in actions rather than keys. Held lasts as long
// as the key is down. Pressed is set when the key goes down and is cleared at the end of the
// frame, so a tap that starts and ends inside one frame is still seen.
struct ComponentInput {
    bool Held(InputAction action) const { return m_held[static_cast<size_t>(action)]; }
    bool Pressed(InputAction action) const { return m_pressed[static_cast<size_t>(action)]; }

    std::array<bool, kInputActionCount> m_held{};
    std::array<bool, kInputActionCount> m_pressed{};
};

class SystemInput {
public:
    // Turns a key event into action state. This is the only place in the game that sees a key.
    static void OnKey(moth::ecs::World& registry, InputData const& bindings,
                      moth::ui::EventKey const& event);

    // Applies the action state to the components that act on it.
    static void Update(GameWorld& world, uint32_t ticks);

    // Clears the presses. Runs after every other system, so each one sees the same frame.
    static void EndFrame(GameWorld& world);
};
