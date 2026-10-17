#include "system_score.h"
#include "gamedata.h"
#include "gamedata_player.h"
#include "system_floating_text.h"
#include "system_hud.h"
#include "system_movement.h"
#include "system_respawn.h"
#include "system_sound.h"
#include "tags.h"

namespace {
    uint32_t const ExtraLifeTextDuration = 1000;
}

void SystemScore::Add(moth::ecs::World& registry, int32_t points, GameData const& gamedata) {
    auto const* playerData = gamedata.GetPlayer();
    if (points <= 0 || playerData == nullptr) {
        return;
    }

    int32_t livesEarned = 0;
    auto view = registry.View<ComponentRun>();
    for (auto [entity, run] : view.each()) {
        run.m_score += points;
        if (playerData->extra_life_score <= 0) {
            break;
        }
        run.m_extraLifeProgress += points;
        while (run.m_extraLifeProgress >= playerData->extra_life_score) {
            run.m_extraLifeProgress -= playerData->extra_life_score;
            // a life earned with the most lives already held is lost
            if (run.m_lives < run.m_maxLives) {
                run.m_lives += 1;
                livesEarned += 1;
            }
        }
        break;
    }

    if (livesEarned == 0) {
        return;
    }
    SystemSound::Play(registry, "extra_life");
    SystemHud::FlashNewLife(registry);
    // the text rises from the ship. it is made after the views are done, because it adds to their pools
    moth::gfx::FloatVec2 shipPosition;
    bool hasShip = false;
    auto playerView = registry.View<PlayerTag, ComponentPosition>(moth::ecs::Exclude<DeadTag>);
    for (auto [entity, position] : playerView.each()) {
        shipPosition = position.m_position;
        hasShip = true;
    }
    if (hasShip) {
        SystemFloatingText::Create(registry, "1UP", shipPosition, ExtraLifeTextDuration);
    }
}

int32_t SystemScore::GetScore(moth::ecs::World const& registry) {
    auto view = registry.View<ComponentRun>();
    for (auto [entity, run] : view.each()) {
        return run.m_score;
    }
    return 0;
}
