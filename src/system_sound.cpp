#include "system_sound.h"
#include "game_world.h"
#include "sound_player.h"
#include <set>

void SystemSound::Play(moth::ecs::World& registry, std::string const& name) {
    if (name.empty()) {
        return;
    }
    auto view = registry.View<ComponentSoundQueue>();
    for (auto [entity, queue] : view.each()) {
        queue.m_requests.push_back(name);
        return;
    }
}

void SystemSound::Update(GameWorld& world, uint32_t /*ticks*/) {
    auto view = world.GetRegistry().View<ComponentSoundQueue>();
    for (auto [entity, queue] : view.each()) {
        // the same sound asked for many times in one frame plays once: copies started together only add up
        // to one louder sound
        std::set<std::string> played;
        for (auto const& name : queue.m_requests) {
            if (played.insert(name).second) {
                world.GetSounds().Play(name);
            }
        }
        queue.m_requests.clear();
    }
}
