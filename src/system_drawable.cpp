#include "system_drawable.h"
#include "component_body.h"
#include "component_entity.h"
#include "system_behaviour.h"
#include "system_movement.h"
#include <moth/ecs/world.h>
#include <moth/graphics/utils/math_utils.h>
#include <spdlog/spdlog.h>
#include "game_world.h"
#include "tags.h"
#include "utils.h"
#include "utils_paths.h"

ComponentSprite::ComponentSprite(SpriteData const& data)
    : m_image(data.image)
    , m_scale(data.scale)
    , m_blendMode(data.blend_mode)
    , m_color(data.color)
    , m_rotation(data.rotation)
    , m_zOrder(data.z_order) {
    if (data.animSet) {
        auto const& set = *data.animSet;
        for (auto const& state : set.states) {
            m_stateIds.insert(state.id);
        }
        m_animator = std::make_shared<moth::anim::Animator>(data.sheet, set);
    } else if (data.sheet) {
        // A sheet entry has no states of its own: synthesise a one-state set
        // around its clip so it runs through the same animator path.
        moth::anim::AnimSet set;
        set.initial = data.clip;
        set.states.push_back({ data.clip, data.clip, std::nullopt });
        m_stateIds.insert(set.initial);
        m_animator = std::make_shared<moth::anim::Animator>(data.sheet, std::move(set));
    }
}

void SetSpriteState(ComponentSprite& sprite, std::string_view state) {
    if (sprite.m_animator == nullptr) {
        return;
    }
    if (sprite.m_stateIds.find(std::string(state)) == sprite.m_stateIds.end()) {
        return;
    }
    if (sprite.m_animator->GetCurrentState() != state) {
        sprite.m_animator->TransitionTo(state);
    }
}

moth::ecs::Entity SystemDrawable::CreateOneShotSprite(moth::ecs::World& registry, SpriteData const& data,
                                                      moth::gfx::FloatVec2 const& position) {
    auto entity = registry.Create();
    registry.Emplace<ComponentPosition>(entity, position);
    auto const& sprite = registry.Emplace<ComponentSprite>(entity, data);
    if (sprite.m_animator == nullptr) {
        // A static sprite has no animation to end on, so it would never die.
        spdlog::error("A one-shot sprite needs a sheet or an anim set");
        registry.Destroy(entity);
        return moth::ecs::kNullEntity;
    }
    registry.Emplace<OneShotTag>(entity);
    return entity;
}

struct DrawImage {
    ComponentSprite const* m_sprite = nullptr;
    moth::gfx::Image m_frame;
    moth::gfx::IntVec2 m_position;
    float m_angle = 0;
    moth::gfx::FloatVec2 m_pivot{ 0.5f, 0.5f };
    bool m_flipX = false;
};

CameraShake::CameraShake(float intensity, uint32_t duration)
    : m_intensity(intensity)
    , m_duration(duration) {
    m_timeOffset.x = Random(0.0f, moth::gfx::F_PI);
    m_timeOffset.y = Random(0.0f, moth::gfx::F_PI);
}


bool CameraShake::Update(uint32_t ticks) {
    m_ticks = std::min(m_duration, m_ticks + ticks);
    auto const factor = static_cast<float>(m_ticks) / static_cast<float>(m_duration);
    auto const curIntensity = moth::gfx::Interp(m_intensity, 0.0f, factor, moth::gfx::InterpType::Smooth);

    auto const xTime = m_timeOffset.x + (factor * moth::gfx::F_PI * 20.0f);
    auto const yTime = m_timeOffset.y + (factor * moth::gfx::F_PI * 10.0f);

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

// void DrawDebugCurves(moth::ecs::World& registry, moth::gfx::IGraphics& graphics) {
//     auto view = registry.View<ComponentBehaviour, ComponentCurveCache>();
//
//     for (auto [entity, behaviour, curve] : view.each()) {
//         graphics.SetColor(moth::gfx::BasicColors::Red);
//         for (size_t i = 1; i < curve.samples.size(); ++i) {
//             auto const offs = moth::gfx::FloatVec2{ behaviour.m_offset.x, behaviour.m_offset.y };
//             auto const p1 =
//                 moth::gfx::FloatVec2{ curve.samples[i - 1].position.x, curve.samples[i - 1].position.y };
//             auto const p2 = moth::gfx::FloatVec2{ curve.samples[i].position.x, curve.samples[i].position.y };
//             graphics.DrawLineF(offs + p1, offs + p2);
//         }
//     }
// }
//
// void DrawDebugSplines(moth::ecs::World& registry, moth::gfx::IGraphics& graphics) {
//     auto view = registry.View<ComponentBehaviour, ComponentSplineCache>();
//
//     auto const samples = 100;
//     auto const tStep = 1.0f / static_cast<float>(samples);
//     graphics.SetColor(moth::gfx::FromARGB(0xFFFFFF00));
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

void DrawDebugBodies(moth::ecs::World& registry, moth::gfx::IGraphics& graphics) {
    auto view = registry.View<ComponentBody, ComponentPosition>();

    graphics.SetColor(moth::gfx::BasicColors::Green);
    for (auto [entity, body, position] : view.each()) {
        moth::gfx::FloatVec2 basis{ 0.0f, body.m_radius };
        moth::gfx::FloatVec2 startPos = basis;
        auto const segments = 50;
        for (int i = 0; i <= segments; ++i) {
            float const angle = (360.0f / segments) * static_cast<float>(i);
            auto const endPos = moth::gfx::Rotate2D(basis, moth::gfx::DegToRad(angle));
            graphics.DrawLineF(position.m_position + startPos, position.m_position + endPos);
            startPos = endPos;
        }
    }
}

void DrawDebugs(moth::ecs::World& registry, moth::gfx::IGraphics& graphics) {
    // TODO: canyon line drawing is a bit broken
    // DrawDebugCurves(registry, graphics);
    // DrawDebugBodies(registry, graphics);
    // DrawDebugSplines(registry, graphics); 
}

void SystemDrawable::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();

    auto cameraView = registry.View<ComponentCamera>();
    if (cameraView.empty()) {
        return;
    }

    auto& camera = cameraView.get<ComponentCamera>(*cameraView.begin());
    UpdateCameraShake(camera, ticks);

    auto const dt = static_cast<float>(ticks) / 1000.0f;
    auto view = registry.View<ComponentSprite>(moth::ecs::Exclude<DeadTag>);
    for (auto [entity, sprite] : view.each()) {
        if (sprite.m_flashTime > 0) {
            sprite.m_flashTime -= dt;
        }
    }
}

void SystemDrawable::Draw(GameWorld& world, moth::gfx::IGraphics& graphics) {
    auto& registry = world.GetRegistry();

    auto cameraView = registry.View<ComponentCamera>();
    if (cameraView.empty()) {
        return;
    }

    auto& camera = cameraView.get<ComponentCamera>(*cameraView.begin());
    auto const cameraOffset = camera.m_offset + camera.m_shakeOffset;

    auto view = registry.View<ComponentSprite, ComponentPosition>();

    std::vector<DrawImage> m_blendedDraws;

    for (auto [entity, sprite, pos] : view.each()) {
        float angle = 0;
        if (auto const* details = registry.TryGet<ComponentEntity>(entity)) {
            angle = details->m_angle;
        }
        auto const fullAngle = angle + sprite.m_rotation;
        auto position = static_cast<moth::gfx::IntVec2>(pos.m_position + cameraOffset);

        DrawImage draw;
        draw.m_sprite = &sprite;
        draw.m_position = position;
        draw.m_angle = fullAngle;
        if (sprite.m_animator) {
            auto const& sp = sprite.m_animator->GetSprite();
            auto const sourceRect = sp.GetCurrentFrameRect();
            auto const pivot = sp.GetCurrentFramePivot();
            draw.m_frame = moth::gfx::Image(sp.GetImage().GetTexture(), sourceRect);
            draw.m_pivot = { static_cast<float>(pivot.x) / static_cast<float>(sp.GetWidth()),
                             static_cast<float>(pivot.y) / static_cast<float>(sp.GetHeight()) };
            draw.m_flipX = sp.GetFlipX();
        } else {
            draw.m_frame = sprite.m_image;
        }
        m_blendedDraws.push_back(draw);
    }

    std::sort(m_blendedDraws.begin(), m_blendedDraws.end(), [](DrawImage const& a, DrawImage const& b) {
        return a.m_sprite->m_zOrder < b.m_sprite->m_zOrder;
    });

    for (auto& draw : m_blendedDraws) {
        auto color = draw.m_sprite->m_color;
        if (draw.m_sprite->m_flashTime > 0) {
            moth::gfx::Color const flashColor{ 1.0, 1.0f, 1.0f, 1.0f };
            float const maxFlashTime = 0.2f;
            float const flashFactor = draw.m_sprite->m_flashTime / maxFlashTime;
            color =
                (color + flashColor * moth::gfx::Interp(0.0f, 1.0f, flashFactor, moth::gfx::InterpType::Linear));
        }
        graphics.SetBlendMode(draw.m_sprite->m_blendMode);
        graphics.SetColor(color);
        moth::gfx::Transform2D const transform{ static_cast<moth::gfx::FloatVec2>(draw.m_position), draw.m_angle,
                                                draw.m_sprite->m_scale };
        graphics.DrawImage(draw.m_frame, transform, draw.m_pivot, draw.m_flipX);
    }

    DrawDebugs(registry, graphics);
}

void SystemDrawable::StartCameraShake(moth::ecs::World& registry, float intensity, uint32_t duration) {
    auto cameraView = registry.View<ComponentCamera>();
    if (!cameraView.empty()) {
        auto& camera = cameraView.get<ComponentCamera>(*cameraView.begin());
        camera.m_shake.emplace_back(CameraShake(intensity, duration));
    }
}
