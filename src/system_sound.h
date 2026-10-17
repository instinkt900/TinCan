#pragma once

#include <moth/ecs/world.h>
#include <cstdint>
#include <string>
#include <vector>

class GameWorld;

// The sounds asked for this frame. One entity holds it, so that code with only the registry to hand can ask for
// a sound without reaching the audio engine.
struct ComponentSoundQueue {
    std::vector<std::string> m_requests;
};

class SystemSound {
public:
    // asks for a sound to play at the end of this frame's update
    static void Play(moth::ecs::World& registry, std::string const& name);
    static void Update(GameWorld& world, uint32_t ticks);
};
