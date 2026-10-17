#include "sound_player.h"
#include "gamedata.h"
#include "gamedata_sound.h"
#include <moth/audio/audio.h>
#include <spdlog/spdlog.h>
#include <algorithm>

SoundPlayer::SoundPlayer(GameData const& gamedata)
    : m_gamedata(gamedata)
    , m_engine(std::make_unique<moth::audio::AudioEngine>()) {
    if (m_engine->IsValid()) {
        m_engine->Start();
    } else {
        spdlog::error("Unable to start the audio engine; the game plays without sound");
    }
}

// the voices are destroyed here, before the engine, as the member order also guarantees
SoundPlayer::~SoundPlayer() {
    m_sounds.clear();
}

void SoundPlayer::Play(std::string const& name) {
    auto* voices = Load(name);
    if (voices == nullptr || voices->m_voices.empty()) {
        return;
    }

    // a free voice if there is one, otherwise the one that started longest ago is restarted
    auto free = std::find_if(voices->m_voices.begin(), voices->m_voices.end(),
                             [](moth::audio::Sound const& voice) { return !voice.IsPlaying(); });
    if (free == voices->m_voices.end()) {
        free = voices->m_voices.begin() + static_cast<std::ptrdiff_t>(voices->m_next);
        voices->m_next = (voices->m_next + 1) % voices->m_voices.size();
        free->Stop();
    }
    free->Play();
}

SoundPlayer::Voices* SoundPlayer::Load(std::string const& name) {
    if (auto const it = m_sounds.find(name); it != m_sounds.end()) {
        return &it->second;
    }
    if (m_failed.count(name) > 0 || !m_engine->IsValid()) {
        return nullptr;
    }

    // Find rather than Get, so that a missing sound is reported here once instead of on every play
    auto const* soundData = m_gamedata.Get<SoundData>()->Find(name);
    if (soundData == nullptr) {
        spdlog::error("Unknown sound {}", name);
        m_failed.insert(name);
        return nullptr;
    }

    Voices voices;
    auto const count = std::max(1, soundData->voices);
    for (int32_t i = 0; i < count; ++i) {
        auto sound = m_engine->LoadSound(soundData->file);
        if (!sound.IsValid()) {
            spdlog::error("Unable to load sound {} from {}", name, soundData->file);
            m_failed.insert(name);
            return nullptr;
        }
        sound.SetVolume(soundData->volume);
        voices.m_voices.push_back(std::move(sound));
    }
    return &m_sounds.emplace(name, std::move(voices)).first->second;
}
