#include "system_drawable.h"
#include "component_body.h"
#include "component_entity.h"
#include "system_behaviour.h"
#include "system_movement.h"
#include <entt/entt.hpp>
#include <canyon/utils/math_utils.h>
#include "game_world.h"
#include "tags.h"
#include "utils.h"

ComponentSprite::ComponentSprite(SpriteData const& data, Affinity affinity)
    : m_image(data.image)
    , m_scale(data.scale)
    , m_blendMode(data.blend_mode)
    , m_color(data.color)
    , m_rotation(data.rotation)
    , m_zOrder(data.z_order) {
    if (data.sheet_data.has_value()) {
        auto const& sheetData = data.sheet_data.value();
        for (int y = 0; y < sheetData.grid_dimensions.y; ++y) {
            for (int x = 0; x < sheetData.grid_dimensions.x; ++x) {
                canyon::IntVec2 cellTL{ sheetData.cell_dimensions.x * x, sheetData.cell_dimensions.y * y };
                canyon::IntVec2 cellBR = cellTL + sheetData.cell_dimensions;
                m_cellRects.emplace_back(canyon::IntRect{ cellTL, cellBR });
            }
        }
    } else {
        m_cellRects.emplace_back(canyon::IntVec2{ 0, 0 },
                                 canyon::IntVec2{ m_image->GetWidth(), m_image->GetHeight() });
    }

    if (affinity != Affinity::None) {
        m_color = affinity == Affinity::Light ? canyon::graphics::BasicColors::Blue
                                              : canyon::graphics::BasicColors::Red;
    }
}

struct DrawImage {
    ComponentSprite const* m_image = nullptr;
    canyon::IntRect m_sourceRect;
    canyon::IntVec2 m_position;
    float m_angle = 0;
};

CameraShake::CameraShake(float intensity, uint32_t duration)
    : m_intensity(intensity)
    , m_duration(duration) {
    m_timeOffset.x = Random(0.0f, canyon::F_PI);
    m_timeOffset.y = Random(0.0f, canyon::F_PI);
}


bool CameraShake::Update(uint32_t ticks) {
    m_ticks = std::min(m_duration, m_ticks + ticks);
    auto const factor = static_cast<float>(m_ticks) / static_cast<float>(m_duration);
    auto const curIntensity = canyon::Interp(m_intensity, 0.0f, factor, canyon::InterpType::Smooth);

    auto const xTime = m_timeOffset.x + (factor * canyon::F_PI * 20.0f);
    auto const yTime = m_timeOffset.y + (factor * canyon::F_PI * 10.0f);

    auto const xOffs = std::sin(xTime) * 5.0f * curIntensity;
    auto const yOffs = std::sin(yTime) * 5.0f * curIntensity;

    m_offset = { xOffs, yOffs };

    return m_ticks < m_duration;
}

void UpdateCameraShake(ComponentCamera& camera, uint32_t ticks) {
    camera.m_shakeOffset = { 0, 0 };
    for (auto it = std::begin(camera.m_shake); it != std::end(camera.m_shake); /* nothing */) {
        auto const active = it->Update(ticks);
        camera.m_shakeOffset += it->GetOffset();
        if (!active) {
            it = camera.m_shake.erase(it);
        } else {
            ++it;
        }
    }
}

void DrawDebugCurves(entt::registry& registry, canyon::graphics::IGraphics& graphics) {
    auto view = registry.view<ComponentBehaviour, ComponentCurveCache>();

    for (auto [entity, behaviour, curve] : view.each()) {
        graphics.SetColor(canyon::graphics::BasicColors::Red);
        for (size_t i = 1; i < curve.samples.size(); ++i) {
            auto const offs = canyon::FloatVec2{ behaviour.m_offset.x, behaviour.m_offset.y };
            auto const p1 =
                canyon::FloatVec2{ curve.samples[i - 1].position.x, curve.samples[i - 1].position.y };
            auto const p2 = canyon::FloatVec2{ curve.samples[i].position.x, curve.samples[i].position.y };
            graphics.DrawLineF(offs + p1, offs + p2);
        }
    }
}

void DrawDebugBodies(entt::registry& registry, canyon::graphics::IGraphics& graphics) {
    auto view = registry.view<ComponentBody, ComponentPosition>();

    graphics.SetColor(canyon::graphics::BasicColors::Green);
    for (auto [entity, body, position] : view.each()) {
        canyon::FloatVec2 basis{ 0.0f, body.m_radius };
        canyon::FloatVec2 startPos = basis;
        auto const segments = 50;
        for (int i = 0; i <= segments; ++i) {
            float const angle = (360.0f / segments) * static_cast<float>(i);
            auto const endPos = canyon::Rotate2D(basis, canyon::Radians(angle));
            graphics.DrawLineF(position.m_position + startPos, position.m_position + endPos);
            startPos = endPos;
        }
    }
}

void DrawDebugs(entt::registry& registry, canyon::graphics::IGraphics& graphics) {
    // TODO: canyon line drawing is a bit broken
    // DrawDebugCurves(registry, graphics);
    DrawDebugBodies(registry, graphics);
}

void SystemDrawable::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();

    auto cameraView = registry.view<ComponentCamera>();
    if (cameraView.empty()) {
        return;
    }

    auto& camera = cameraView.get<ComponentCamera>(*cameraView.begin());
    UpdateCameraShake(camera, ticks);

    auto const dt = static_cast<float>(ticks) / 1000.0f;
    auto view = registry.view<ComponentSprite>(entt::exclude<DeadTag>);
    for (auto [entity, sprite] : view.each()) {
        if (sprite.m_flashTime > 0) {
            sprite.m_flashTime -= dt;
        }
    }
}

void SystemDrawable::Draw(GameWorld& world, canyon::graphics::IGraphics& graphics) {
    auto& registry = world.GetRegistry();

    auto cameraView = registry.view<ComponentCamera>();
    if (cameraView.empty()) {
        return;
    }

    auto& camera = cameraView.get<ComponentCamera>(*cameraView.begin());
    auto const cameraOffset = camera.m_offset + camera.m_shakeOffset;

    auto view = registry.view<ComponentSprite, ComponentPosition>();

    std::vector<DrawImage> m_blendedDraws;

    for (auto [entity, sprite, pos] : view.each()) {
        float angle = 0;
        if (auto const* details = registry.try_get<ComponentEntity>(entity)) {
            angle = details->m_angle;
        }
        auto const fullAngle = angle + sprite.m_rotation;
        auto const sourceRect = sprite.m_cellRects[sprite.m_cellIndex];
        auto position = static_cast<canyon::IntVec2>(pos.m_position + cameraOffset);
        m_blendedDraws.push_back({ &sprite, sourceRect, position, fullAngle });
    }

    std::sort(m_blendedDraws.begin(), m_blendedDraws.end(), [](DrawImage const& a, DrawImage const& b) {
        return a.m_image->m_zOrder < b.m_image->m_zOrder;
    });

    for (auto& draw : m_blendedDraws) {
        auto color = draw.m_image->m_color;
        if (draw.m_image->m_flashTime > 0) {
            canyon::graphics::Color const flashColor{ 1.0, 1.0f, 1.0f, 1.0f };
            float const maxFlashTime = 0.2f;
            float const flashFactor = draw.m_image->m_flashTime / maxFlashTime;
            color = (color + flashColor * canyon::Interp(0.0f, 1.0f, flashFactor, canyon::InterpType::Linear));
        }
        graphics.SetBlendMode(draw.m_image->m_blendMode);
        graphics.SetColor(color);
        auto const baseSize = static_cast<canyon::FloatVec2>(draw.m_sourceRect.dimensions());
        auto const scaledSize = static_cast<canyon::IntVec2>(baseSize * draw.m_image->m_scale);
        canyon::IntVec2 halfSize = scaledSize / 2;
        auto destRect = canyon::MakeRect(draw.m_position.x - halfSize.x, draw.m_position.y - halfSize.y,
                                         scaledSize.x, scaledSize.y);
        graphics.DrawImage(*draw.m_image->m_image, static_cast<canyon::IntRect>(destRect), &draw.m_sourceRect,
                           draw.m_angle);
    }

    DrawDebugs(registry, graphics);
}

void SystemDrawable::StartCameraShake(entt::registry& registry, float intensity, uint32_t duration) {
    auto cameraView = registry.view<ComponentCamera>();
    if (!cameraView.empty()) {
        auto& camera = cameraView.get<ComponentCamera>(*cameraView.begin());
        camera.m_shake.emplace_back(CameraShake(intensity, duration));
    }
}
