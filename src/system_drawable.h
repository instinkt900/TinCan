#pragma once

#include "gamedata_entity.h"
#include "gamedata_sprite.h"
#include <moth/anim/animator.h>
#include <moth/graphics/graphics/igraphics.h>
#include <moth/ecs/world.h>
#include <unordered_set>

class GameWorld;

class CameraShake {
public:
    CameraShake(float intensity, uint32_t duration);
    bool Update(uint32_t ticks);

    moth::gfx::FloatVec2 const& GetOffset() { return m_offset; }

private:
    float m_intensity = 0.0f;
    uint32_t m_ticks = 0;
    uint32_t m_duration = 0;
    moth::gfx::FloatVec2 m_timeOffset;
    moth::gfx::FloatVec2 m_offset;
};

struct ComponentCamera {
    float m_zoom = 1.0f;
    moth::gfx::FloatVec2 m_offset;
    moth::gfx::FloatVec2 m_shakeOffset;
    std::vector<CameraShake> m_shake;
};

struct ComponentSprite {
    moth::gfx::Image m_image;
    std::shared_ptr<moth::anim::Animator> m_animator;
    std::unordered_set<std::string> m_stateIds;
    moth::gfx::FloatVec2 m_scale{ 1.0f, 1.0f };
    moth::gfx::BlendMode m_blendMode = moth::gfx::BlendMode::Replace;
    moth::gfx::Color m_color = moth::gfx::BasicColors::White;
    float m_rotation = 0;
    int32_t m_zOrder = 0;

    float m_flashTime = 0;

    explicit ComponentSprite(SpriteData const& data);
};

// Switches an animated sprite to a named state; a no-op for static sprites and
// for states the sprite does not define (which log a warning).
void SetSpriteState(ComponentSprite& sprite, std::string_view state);

class SystemDrawable {
public:
    static void Update(GameWorld& world, uint32_t ticks);
    static void Draw(GameWorld& world, moth::gfx::IGraphics& graphics);

    static void StartCameraShake(moth::ecs::World& registry, float intensity, uint32_t duration);
};
