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

ComponentSprite::ComponentSprite(SpriteData const& data, float spinScale)
    : m_image(data.image)
    , m_scale(data.scale)
    , m_blendMode(data.blend_mode)
    , m_color(data.color)
    , m_rotation(data.rotation)
    , m_spin(data.spin * spinScale)
    , m_zOrder(data.z_order) {
    if (data.spin_random_phase && m_spin != 0.0f) {
        m_spinAngle = Random(0.0f, moth::gfx::F_PI * 2.0f);
    }
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
    moth::gfx::FloatVec2 m_position;
    float m_angle = 0;
};

void DrawSprite(moth::gfx::IGraphics& graphics, ComponentSprite const& sprite,
                moth::gfx::FloatVec2 const& position, float angle) {
    moth::gfx::Image frame = sprite.m_image;
    moth::gfx::FloatVec2 pivot{ 0.5f, 0.5f };
    bool flipX = false;
    if (sprite.m_animator) {
        auto const& sp = sprite.m_animator->GetSprite();
        auto const sourceRect = sp.GetCurrentFrameRect();
        auto const framePivot = sp.GetCurrentFramePivot();
        frame = moth::gfx::Image(sp.GetImage().GetTexture(), sourceRect);
        pivot = { static_cast<float>(framePivot.x) / static_cast<float>(sp.GetWidth()),
                  static_cast<float>(framePivot.y) / static_cast<float>(sp.GetHeight()) };
        flipX = sp.GetFlipX();
    }

    auto color = sprite.m_color;
    if (sprite.m_flashTime > 0) {
        moth::gfx::Color const flashColor{ 1.0, 1.0f, 1.0f, 1.0f };
        float const maxFlashTime = 0.2f;
        float const flashFactor = sprite.m_flashTime / maxFlashTime;
        color = (color + flashColor * moth::gfx::Interp(0.0f, 1.0f, flashFactor, moth::gfx::InterpType::Linear));
    }
    graphics.SetBlendMode(sprite.m_blendMode);
    graphics.SetColor(color);

    // sprites are drawn on whole pixels so that a moving entity does not shimmer
    auto const snapped = static_cast<moth::gfx::FloatVec2>(static_cast<moth::gfx::IntVec2>(position));
    moth::gfx::Transform2D const transform{ snapped, angle + sprite.m_rotation + sprite.m_spinAngle,
                                            sprite.m_scale };
    graphics.DrawImage(frame, transform, pivot, flipX);
}

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
        if (sprite.m_spin != 0.0f) {
            // wrapped so that a long-lived sprite does not lose precision in its angle
            sprite.m_spinAngle = std::fmod(sprite.m_spinAngle + (sprite.m_spin * dt), moth::gfx::F_PI * 2.0f);
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

        DrawImage draw;
        draw.m_sprite = &sprite;
        draw.m_position = pos.m_position + cameraOffset;
        draw.m_angle = angle;
        m_blendedDraws.push_back(draw);
    }

    std::sort(m_blendedDraws.begin(), m_blendedDraws.end(), [](DrawImage const& a, DrawImage const& b) {
        return a.m_sprite->m_zOrder < b.m_sprite->m_zOrder;
    });

    for (auto& draw : m_blendedDraws) {
        DrawSprite(graphics, *draw.m_sprite, draw.m_position, draw.m_angle);
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
