#include "system_drawable.h"
#include "component_body.h"
#include "component_entity.h"
#include "system_behaviour.h"
#include "system_movement.h"
#include <entt/entt.hpp>
#include <moth_graphics/utils/math_utils.h>
#include "game_world.h"
#include "tags.h"
#include "utils.h"
#include "utils_paths.h"

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
                moth_graphics::IntVec2 cellTL{ sheetData.cell_dimensions.x * x, sheetData.cell_dimensions.y * y };
                moth_graphics::IntVec2 cellBR = cellTL + sheetData.cell_dimensions;
                m_cellRects.emplace_back(moth_graphics::IntRect{ cellTL, cellBR });
            }
        }
    } else {
        m_cellRects.emplace_back(moth_graphics::IntVec2{ 0, 0 },
                                 moth_graphics::IntVec2{ m_image->GetWidth(), m_image->GetHeight() });
    }

    if (affinity != Affinity::None) {
        m_color = affinity == Affinity::Light ? moth_graphics::graphics::BasicColors::Blue
                                              : moth_graphics::graphics::BasicColors::Red;
    }
}

struct DrawImage {
    ComponentSprite const* m_image = nullptr;
    moth_graphics::IntRect m_sourceRect;
    moth_graphics::IntVec2 m_position;
    float m_angle = 0;
};

CameraShake::CameraShake(float intensity, uint32_t duration)
    : m_intensity(intensity)
    , m_duration(duration) {
    m_timeOffset.x = Random(0.0f, moth_graphics::F_PI);
    m_timeOffset.y = Random(0.0f, moth_graphics::F_PI);
}


bool CameraShake::Update(uint32_t ticks) {
    m_ticks = std::min(m_duration, m_ticks + ticks);
    auto const factor = static_cast<float>(m_ticks) / static_cast<float>(m_duration);
    auto const curIntensity = moth_graphics::Interp(m_intensity, 0.0f, factor, moth_graphics::InterpType::Smooth);

    auto const xTime = m_timeOffset.x + (factor * moth_graphics::F_PI * 20.0f);
    auto const yTime = m_timeOffset.y + (factor * moth_graphics::F_PI * 10.0f);

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

// void DrawDebugCurves(entt::registry& registry, moth_graphics::graphics::IGraphics& graphics) {
//     auto view = registry.view<ComponentBehaviour, ComponentCurveCache>();
//
//     for (auto [entity, behaviour, curve] : view.each()) {
//         graphics.SetColor(moth_graphics::graphics::BasicColors::Red);
//         for (size_t i = 1; i < curve.samples.size(); ++i) {
//             auto const offs = moth_graphics::FloatVec2{ behaviour.m_offset.x, behaviour.m_offset.y };
//             auto const p1 =
//                 moth_graphics::FloatVec2{ curve.samples[i - 1].position.x, curve.samples[i - 1].position.y };
//             auto const p2 = moth_graphics::FloatVec2{ curve.samples[i].position.x, curve.samples[i].position.y };
//             graphics.DrawLineF(offs + p1, offs + p2);
//         }
//     }
// }
//
// void DrawDebugSplines(entt::registry& registry, moth_graphics::graphics::IGraphics& graphics) {
//     auto view = registry.view<ComponentBehaviour, ComponentSplineCache>();
//
//     auto const samples = 100;
//     auto const tStep = 1.0f / static_cast<float>(samples);
//     graphics.SetColor(moth_graphics::graphics::FromARGB(0xFFFFFF00));
//     for (auto [entity, behaviour, spline] : view.each()) {
//         auto t = 0.0f;
//         auto lastSample = SampleCatmullRomPath(behaviour.m_spline, t);
//         for (auto i = 1; i <= samples; ++i) {
//             t = tStep * static_cast<float>(i);
//             auto const newSample = SampleCatmullRomPath(behaviour.m_spline, t);
//             graphics.DrawLineF(lastSample, newSample);
//             lastSample = newSample;
//         }
//     }
// }

void DrawDebugBodies(entt::registry& registry, moth_graphics::graphics::IGraphics& graphics) {
    auto view = registry.view<ComponentBody, ComponentPosition>();

    graphics.SetColor(moth_graphics::graphics::BasicColors::Green);
    for (auto [entity, body, position] : view.each()) {
        moth_graphics::FloatVec2 basis{ 0.0f, body.m_radius };
        moth_graphics::FloatVec2 startPos = basis;
        auto const segments = 50;
        for (int i = 0; i <= segments; ++i) {
            float const angle = (360.0f / segments) * static_cast<float>(i);
            auto const endPos = moth_graphics::Rotate2D(basis, moth_graphics::Radians(angle));
            graphics.DrawLineF(position.m_position + startPos, position.m_position + endPos);
            startPos = endPos;
        }
    }
}

void DrawDebugs(entt::registry& registry, moth_graphics::graphics::IGraphics& graphics) {
    // TODO: canyon line drawing is a bit broken
    // DrawDebugCurves(registry, graphics);
    // DrawDebugBodies(registry, graphics);
    // DrawDebugSplines(registry, graphics); 
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

void SystemDrawable::Draw(GameWorld& world, moth_graphics::graphics::IGraphics& graphics) {
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
        auto position = static_cast<moth_graphics::IntVec2>(pos.m_position + cameraOffset);
        m_blendedDraws.push_back({ &sprite, sourceRect, position, fullAngle });
    }

    std::sort(m_blendedDraws.begin(), m_blendedDraws.end(), [](DrawImage const& a, DrawImage const& b) {
        return a.m_image->m_zOrder < b.m_image->m_zOrder;
    });

    for (auto& draw : m_blendedDraws) {
        auto color = draw.m_image->m_color;
        if (draw.m_image->m_flashTime > 0) {
            moth_graphics::graphics::Color const flashColor{ 1.0, 1.0f, 1.0f, 1.0f };
            float const maxFlashTime = 0.2f;
            float const flashFactor = draw.m_image->m_flashTime / maxFlashTime;
            color =
                (color + flashColor * moth_graphics::Interp(0.0f, 1.0f, flashFactor, moth_graphics::InterpType::Linear));
        }
        graphics.SetBlendMode(draw.m_image->m_blendMode);
        graphics.SetColor(color);
        auto const baseSize = static_cast<moth_graphics::FloatVec2>(draw.m_sourceRect.dimensions());
        auto const scaledSize = static_cast<moth_graphics::IntVec2>(baseSize * draw.m_image->m_scale);
        moth_graphics::IntVec2 halfSize = scaledSize / 2;
        auto destRect = moth_graphics::MakeRect(draw.m_position.x - halfSize.x, draw.m_position.y - halfSize.y,
                                         scaledSize.x, scaledSize.y);
        graphics.DrawImage(*draw.m_image->m_image, static_cast<moth_graphics::IntRect>(destRect), &draw.m_sourceRect,
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
