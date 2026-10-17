#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

class GameData;

namespace moth::audio {
    class AudioEngine;
    class Sound;
}

// Plays sound effects by their data entry. Each sound is loaded the first time it plays, as a set of voices so
// that several copies can overlap. A sound that cannot be found or loaded is logged once and then stays silent.
class SoundPlayer {
public:
    explicit SoundPlayer(GameData const& gamedata);
    ~SoundPlayer();

    void Play(std::string const& name);

    SoundPlayer(SoundPlayer const&) = delete;
    SoundPlayer(SoundPlayer&&) = delete;
    SoundPlayer& operator=(SoundPlayer const&) = delete;
    SoundPlayer& operator=(SoundPlayer&&) = delete;

private:
    struct Voices {
        std::vector<moth::audio::Sound> m_voices;
        size_t m_next = 0; // the voice to take over when every voice is busy
    };

    Voices* Load(std::string const& name);

    GameData const& m_gamedata;
    std::unique_ptr<moth::audio::AudioEngine> m_engine;
    // declared after the engine, so the sounds are destroyed before the engine that made them
    std::map<std::string, Voices> m_sounds;
    std::set<std::string> m_failed;
};
