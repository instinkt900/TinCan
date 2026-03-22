#pragma once

#include "gamedata_entity.h"
#include "gamedata_sprite.h"
#include <moth_graphics/graphics/igraphics.h>
#include <entt/entity/fwd.hpp>

class GameWorld;

class CameraShake {
public:
    CameraShake(float intensity, uint32_t duration);
    bool Update(uint32_t ticks);

    moth_graphics::FloatVec2 const& GetOffset() { return m_offset; }

private:
    float m_intensity = 0.0f;
    uint32_t m_ticks = 0;
    uint32_t m_duration = 0;
    moth_graphics::FloatVec2 m_timeOffset;
    moth_graphics::FloatVec2 m_offset;
};

struct ComponentCamera {
    float m_zoom = 1.0f;
    moth_graphics::FloatVec2 m_offset;
    moth_graphics::FloatVec2 m_shakeOffset;
    std::vector<CameraShake> m_shake;
};

struct ComponentSprite {
    std::shared_ptr<moth_graphics::graphics::IImage> m_image;
    moth_graphics::FloatVec2 m_scale{ 1.0f, 1.0f };
    moth_graphics::graphics::BlendMode m_blendMode = moth_graphics::graphics::BlendMode::Replace;
    moth_graphics::graphics::Color m_color = moth_graphics::graphics::BasicColors::White;
    float m_rotation = 0;
    int32_t m_zOrder = 0;

    std::vector<moth_graphics::IntRect> m_cellRects;
    int32_t m_cellIndex = 0;

    float m_fps = 0;
    float m_animationTime = 0;

    float m_flashTime = 0;

    explicit ComponentSprite(SpriteData const& data, Affinity affinity = Affinity::None);
};

class SystemDrawable {
public:
    static void Update(GameWorld& world, uint32_t ticks);
    static void Draw(GameWorld& world, moth_graphics::graphics::IGraphics& graphics);

    static void StartCameraShake(entt::registry& registry, float intensity, uint32_t duration);
};
