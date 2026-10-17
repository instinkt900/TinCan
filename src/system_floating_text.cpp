#include "system_floating_text.h"
#include "game_world.h"
#include "system_movement.h"
#include "tags.h"
#include <algorithm>

namespace {
    // pixels per second the text rises
    float const RiseSpeed = 60.0f;
    // the box the text is centred in, wide enough for a weapon name and level
    int const TextWidth = 400;
    int const TextHeight = 40;
}

void SystemFloatingText::Create(moth::ecs::World& registry, std::string text, moth::gfx::FloatVec2 const& position,
                                uint32_t duration) {
    auto const entity = registry.Create();
    registry.Emplace<ComponentPosition>(entity, position);
    auto& floating = registry.Emplace<ComponentFloatingText>(entity);
    floating.m_text = std::move(text);
    floating.m_msLeft = duration;
}

void SystemFloatingText::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto const dt = static_cast<float>(ticks) / 1000.0f;
    auto view = registry.View<ComponentFloatingText, ComponentPosition>(moth::ecs::Exclude<DeadTag>);
    for (auto [entity, floating, position] : view.each()) {
        position.m_lastPosition = position.m_position;
        position.m_position.y -= RiseSpeed * dt;
        floating.m_msLeft -= std::min(floating.m_msLeft, ticks);
        if (floating.m_msLeft == 0) {
            registry.GetOrEmplace<DeadTag>(entity);
        }
    }
}

void SystemFloatingText::Draw(GameWorld& world, moth::gfx::IGraphics& graphics, moth::gfx::IFont& font) {
    auto& registry = world.GetRegistry();
    graphics.SetBlendMode(moth::gfx::BlendMode::Alpha);
    graphics.SetColor(moth::gfx::BasicColors::White);
    auto view = registry.View<ComponentFloatingText, ComponentPosition>(moth::ecs::Exclude<DeadTag>);
    for (auto [entity, floating, position] : view.each()) {
        auto const centre = static_cast<moth::gfx::IntVec2>(position.m_position);
        auto const rect = moth::gfx::MakeRect(centre.x - (TextWidth / 2), centre.y - (TextHeight / 2), TextWidth,
                                              TextHeight);
        graphics.DrawText(floating.m_text, font, rect, moth::gfx::TextHorizAlignment::Center,
                          moth::gfx::TextVertAlignment::Middle);
    }
}
