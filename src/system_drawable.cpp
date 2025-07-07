#include "system_drawable.h"
#include "system_behaviour.h"
#include "system_movement.h"
#include <entt/entt.hpp>

struct DrawImage {
    SpriteImage const* m_image = nullptr;
    moth_ui::IntVec2 m_position;
};

void DrawDebugCurves(entt::registry& registry, canyon::graphics::IGraphics& graphics) {
    auto view = registry.view<ComponentBehaviour, ComponentCurveCache>();

    for (auto [entity, behaviour, curve]: view.each()) {
        graphics.SetColor(canyon::graphics::BasicColors::Red);
        for (size_t i = 1; i < curve.samples.size(); ++i) {
            auto const offs = canyon::FloatVec2{ behaviour.m_offset.x, behaviour.m_offset.y };
            auto const p1 = canyon::FloatVec2{ curve.samples[i-1].position.x, curve.samples[i-1].position.y };
            auto const p2 = canyon::FloatVec2{ curve.samples[i].position.x, curve.samples[i].position.y };
            graphics.DrawLineF(offs+p1, offs+p2);
        }
    }
}

void DrawDebugs(entt::registry& registry, canyon::graphics::IGraphics& graphics) {
    DrawDebugCurves(registry, graphics);
}

void SystemDrawable::Update(entt::registry& registry, canyon::graphics::IGraphics& graphics) {
    auto view = registry.view<ComponentDrawable, ComponentPosition>();

    std::vector<DrawImage> m_opaqueDraws;
    std::vector<DrawImage> m_blendedDraws;

    for (auto [entity, drawable, pos] : view.each()) {
        for (auto& [imageName, spriteImage] : drawable.m_spriteData.images) {
            auto position = static_cast<moth_ui::IntVec2>(pos.m_position);
            if (spriteImage.blend_mode == canyon::graphics::BlendMode::Replace) {
                m_opaqueDraws.push_back({ &spriteImage, position });
            } else {
                m_blendedDraws.push_back({ &spriteImage, position });
            }
        }
    }

    std::sort(m_opaqueDraws.begin(), m_opaqueDraws.end(),
              [](DrawImage const& a, DrawImage const& b) { return a.m_image->zOrder < b.m_image->zOrder; });

    std::sort(m_blendedDraws.begin(), m_blendedDraws.end(),
              [](DrawImage const& a, DrawImage const& b) { return a.m_image->zOrder < b.m_image->zOrder; });

    graphics.SetBlendMode(canyon::graphics::BlendMode::Replace);
    for (auto& draw : m_opaqueDraws) {
        graphics.SetColor(draw.m_image->color);
        auto const baseSize =
            moth_ui::FloatVec2{ draw.m_image->image->GetWidth(), draw.m_image->image->GetHeight() };
        auto const scaledSize = static_cast<moth_ui::IntVec2>(baseSize * draw.m_image->scale);
        moth_ui::IntVec2 halfSize = scaledSize / 2;
        auto destRect = canyon::MakeRect(draw.m_position.x - halfSize.x, draw.m_position.y - halfSize.y,
                                         scaledSize.x, scaledSize.y);
        graphics.DrawImage(*draw.m_image->image, static_cast<canyon::IntRect>(destRect), nullptr);
    }

    for (auto& draw : m_blendedDraws) {
        graphics.SetBlendMode(draw.m_image->blend_mode);
        graphics.SetColor(draw.m_image->color);
        auto const baseSize =
            moth_ui::FloatVec2{ draw.m_image->image->GetWidth(), draw.m_image->image->GetHeight() };
        auto const scaledSize = static_cast<moth_ui::IntVec2>(baseSize * draw.m_image->scale);
        moth_ui::IntVec2 halfSize = scaledSize / 2;
        auto destRect = canyon::MakeRect(draw.m_position.x - halfSize.x, draw.m_position.y - halfSize.y,
                                         scaledSize.x, scaledSize.y);
        graphics.DrawImage(*draw.m_image->image, static_cast<canyon::IntRect>(destRect), nullptr);
    }

    DrawDebugs(registry, graphics);
}
