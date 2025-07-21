#pragma once

#include "gamedata_sprite.h"
#include <canyon/graphics/igraphics.h>
#include <entt/entity/fwd.hpp>

class GameWorld;

class CameraShake {
public:
    CameraShake(float intensity, uint32_t duration);
    bool Update(uint32_t ticks);

    canyon::FloatVec2 const& GetOffset() { return m_offset; }

private:
    float m_intensity = 0.0f;
    uint32_t m_ticks = 0;
    uint32_t m_duration = 0;
    canyon::FloatVec2 m_timeOffset;
    canyon::FloatVec2 m_offset;
};

struct ComponentCamera {
    float m_zoom = 1.0f;
    canyon::FloatVec2 m_offset;
    canyon::FloatVec2 m_shakeOffset;
    std::vector<CameraShake> m_shake;
};

struct ComponentDrawable {
    SpriteData m_spriteData;
};

class SystemDrawable {
public:
    static void Update(GameWorld& world, uint32_t ticks);
    static void Draw(GameWorld& world, canyon::graphics::IGraphics& graphics);

    static void StartCameraShake(entt::registry& registry, float intensity, uint32_t duration);
};
