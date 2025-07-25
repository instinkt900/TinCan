#include "system_drawable.h"
#include "component_entity.h"
#include "system_behaviour.h"
#include "system_movement.h"
#include <entt/entt.hpp>
#include <canyon/utils/math_utils.h>
#include "game_world.h"
#include "utils.h"

DrawableSprite::DrawableSprite(SpriteImage const& data)
    : m_image(data.image)
    , m_scale(data.scale)
    , m_offset(data.offset)
    , m_rotation(data.rotation)
    , m_blendMode(data.blend_mode)
    , m_color(data.color)
    , m_zOrder(data.zOrder) {
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
}

ComponentDrawable::ComponentDrawable(SpriteData const& data) {
    for (auto const& [name, image] : data.images) {
        m_sprites.insert(std::make_pair(name, DrawableSprite(image)));
    }
}

struct DrawImage {
    DrawableSprite const* m_image = nullptr;
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

void DrawDebugs(entt::registry& registry, canyon::graphics::IGraphics& graphics) {
    // TODO: canyon line drawing is a bit broken
    // DrawDebugCurves(registry, graphics);
}

void SystemDrawable::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();

    auto cameraView = registry.view<ComponentCamera>();
    if (cameraView.empty()) {
        return;
    }

    auto& camera = cameraView.get<ComponentCamera>(*cameraView.begin());
    UpdateCameraShake(camera, ticks);
}

void SystemDrawable::Draw(GameWorld& world, canyon::graphics::IGraphics& graphics) {
    auto& registry = world.GetRegistry();

    auto cameraView = registry.view<ComponentCamera>();
    if (cameraView.empty()) {
        return;
    }

    auto& camera = cameraView.get<ComponentCamera>(*cameraView.begin());
    auto const cameraOffset = camera.m_offset + camera.m_shakeOffset;

    auto view = registry.view<ComponentDrawable, ComponentPosition>();

    std::vector<DrawImage> m_opaqueDraws;
    std::vector<DrawImage> m_blendedDraws;

    for (auto [entity, drawable, pos] : view.each()) {
        float angle = 0;
        if (auto const* details = registry.try_get<ComponentEntity>(entity)) {
            angle = details->m_angle;
        }
        for (auto& [imageName, spriteImage] : drawable.m_sprites) {
            auto const fullAngle = angle + spriteImage.m_rotation;
            auto const sourceRect = spriteImage.m_cellRects[spriteImage.m_cellIndex];
            auto position = static_cast<canyon::IntVec2>(pos.m_position + cameraOffset);
            if (spriteImage.m_blendMode == canyon::graphics::BlendMode::Replace) {
                m_opaqueDraws.push_back({ &spriteImage, sourceRect, position, fullAngle });
            } else {
                m_blendedDraws.push_back({ &spriteImage, sourceRect, position, fullAngle });
            }
        }
    }

    std::sort(m_opaqueDraws.begin(), m_opaqueDraws.end(),
              [](DrawImage const& a, DrawImage const& b) { return a.m_image->m_zOrder < b.m_image->m_zOrder; });

    std::sort(m_blendedDraws.begin(), m_blendedDraws.end(),
              [](DrawImage const& a, DrawImage const& b) { return a.m_image->m_zOrder < b.m_image->m_zOrder; });

    graphics.SetBlendMode(canyon::graphics::BlendMode::Replace);
    for (auto& draw : m_opaqueDraws) {
        graphics.SetColor(draw.m_image->m_color);
        auto const baseSize = static_cast<canyon::FloatVec2>(draw.m_sourceRect.dimensions());
        auto const scaledSize = static_cast<canyon::IntVec2>(baseSize * draw.m_image->m_scale);
        canyon::IntVec2 halfSize = scaledSize / 2;
        auto destRect = canyon::MakeRect(draw.m_position.x - halfSize.x, draw.m_position.y - halfSize.y,
                                         scaledSize.x, scaledSize.y);
        graphics.DrawImage(*draw.m_image->m_image, static_cast<canyon::IntRect>(destRect), &draw.m_sourceRect,
                           draw.m_angle);
    }

    for (auto& draw : m_blendedDraws) {
        graphics.SetBlendMode(draw.m_image->m_blendMode);
        graphics.SetColor(draw.m_image->m_color);
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
