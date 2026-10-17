#include "system_hud.h"
#include "game_world.h"
#include "gamedata_player.h"
#include "system_respawn.h"
#include "system_shield.h"
#include "tags.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace {
    int const Margin = 16;

    float const LifeIconScale = 0.6f;
    float const LifeIconSpacing = 36.0f;
    uint32_t const LifeFlashDuration = 1000;
    uint32_t const LifeFlashInterval = 100;

    int const ScoreWidth = 400;
    int const ScoreHeight = 40;

    int const SegmentCount = 10;
    float const SegmentWidth = 28.0f;
    float const SegmentHeight = 12.0f;
    float const SegmentGap = 4.0f;
    // how fast full segments pulse, in milliseconds per pulse
    float const GlowPeriod = 600.0f;

    ComponentHud* FindHud(moth::ecs::World& registry) {
        auto view = registry.View<ComponentHud>();
        for (auto [entity, hud] : view.each()) {
            return &hud;
        }
        return nullptr;
    }

    void DrawLives(GameWorld& world, moth::gfx::IGraphics& graphics, ComponentHud& hud) {
        auto const& gamedata = world.GetGameData();
        if (hud.m_lifeIcon == nullptr) {
            auto const* playerData = gamedata.GetPlayer();
            if (playerData == nullptr || !playerData->sprite.valid()) {
                return;
            }
            hud.m_lifeIcon = std::make_unique<ComponentSprite>(*playerData->sprite);
            hud.m_lifeIcon->m_scale = { LifeIconScale, LifeIconScale };
        }

        auto const lives = SystemRespawn::GetLives(world.GetRegistry());
        for (int32_t i = 0; i < lives; ++i) {
            bool const newest = i == lives - 1;
            if (newest && hud.m_lifeFlashLeft > 0 && (hud.m_lifeFlashLeft / LifeFlashInterval) % 2 == 0) {
                continue;
            }
            auto const position = moth::gfx::FloatVec2{ static_cast<float>(Margin) + (LifeIconSpacing / 2.0f) +
                                                            (LifeIconSpacing * static_cast<float>(i)),
                                                        static_cast<float>(Margin) + (LifeIconSpacing / 2.0f) };
            DrawSprite(graphics, *hud.m_lifeIcon, position, 0.0f);
        }
    }

    void DrawScore(GameWorld& world, moth::gfx::IGraphics& graphics, moth::gfx::IFont& font) {
        auto const width = world.GetWorldSize().x;
        graphics.SetBlendMode(moth::gfx::BlendMode::Alpha);
        graphics.SetColor(moth::gfx::BasicColors::White);
        auto const rect = moth::gfx::MakeRect(width - Margin - ScoreWidth, Margin, ScoreWidth, ScoreHeight);
        graphics.DrawText(std::to_string(world.GetPlayerScore()), font, rect, moth::gfx::TextHorizAlignment::Right,
                          moth::gfx::TextVertAlignment::Middle);
    }

    void DrawPowerMeter(GameWorld& world, moth::gfx::IGraphics& graphics, ComponentHud const& hud) {
        auto& registry = world.GetRegistry();
        float power = 0.0f;
        float maxPower = 0.0f;
        auto view = registry.View<PlayerTag, ComponentPower>(moth::ecs::Exclude<DeadTag>);
        for (auto [entity, playerPower] : view.each()) {
            power = playerPower.m_power;
            maxPower = playerPower.m_maxPower;
        }
        if (maxPower <= 0.0f) {
            return;
        }

        auto const segmentPower = maxPower / static_cast<float>(SegmentCount);
        // full segments pulse between bright and brighter, so a full one never looks like a filling one
        auto const phase = static_cast<float>(hud.m_time % static_cast<uint32_t>(GlowPeriod)) / GlowPeriod;
        auto const glow = 0.8f + (0.2f * std::sin(phase * 2.0f * moth::gfx::F_PI));
        auto const top = static_cast<float>(world.GetWorldSize().y - Margin) - SegmentHeight;

        graphics.SetBlendMode(moth::gfx::BlendMode::Alpha);
        for (int i = 0; i < SegmentCount; ++i) {
            auto const left = static_cast<float>(Margin) + ((SegmentWidth + SegmentGap) * static_cast<float>(i));
            auto const fill = std::clamp((power - (segmentPower * static_cast<float>(i))) / segmentPower, 0.0f, 1.0f);

            graphics.SetColor(moth::gfx::Color{ 1.0f, 1.0f, 1.0f, 0.25f });
            graphics.DrawFillRectF(moth::gfx::MakeRect(left, top, SegmentWidth, SegmentHeight));
            if (fill >= 1.0f) {
                graphics.SetColor(moth::gfx::Color{ 1.0f, 0.9f, 0.3f, glow });
                graphics.DrawFillRectF(moth::gfx::MakeRect(left, top, SegmentWidth, SegmentHeight));
            } else if (fill > 0.0f) {
                graphics.SetColor(moth::gfx::Color{ 1.0f, 0.9f, 0.3f, 0.5f });
                graphics.DrawFillRectF(moth::gfx::MakeRect(left, top, SegmentWidth * fill, SegmentHeight));
            }
        }
        graphics.SetColor(moth::gfx::BasicColors::White);
    }
}

void SystemHud::FlashNewLife(moth::ecs::World& registry) {
    if (auto* hud = FindHud(registry)) {
        hud->m_lifeFlashLeft = LifeFlashDuration;
    }
}

void SystemHud::Update(GameWorld& world, uint32_t ticks) {
    if (auto* hud = FindHud(world.GetRegistry())) {
        hud->m_time += ticks;
        hud->m_lifeFlashLeft -= std::min(hud->m_lifeFlashLeft, ticks);
    }
}

void SystemHud::Draw(GameWorld& world, moth::gfx::IGraphics& graphics, moth::gfx::IFont& font) {
    auto* hud = FindHud(world.GetRegistry());
    if (hud == nullptr) {
        return;
    }
    DrawLives(world, graphics, *hud);
    DrawScore(world, graphics, font);
    DrawPowerMeter(world, graphics, *hud);
}
