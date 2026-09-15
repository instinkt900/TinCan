#include "system_explosion.h"
#include "system_drawable.h"
#include "system_movement.h"
#include "tags.h"
#include "game_world.h"

moth::ecs::Entity SystemExplosion::CreateExplosion(moth::ecs::World& registry,
                                                   std::shared_ptr<moth::gfx::SpriteSheet> const& sheet,
                                                   moth::gfx::FloatVec2 const& position) {
    if (sheet == nullptr) {
        return moth::ecs::kNullEntity;
    }
    auto entity = registry.Create();
    registry.Emplace<ComponentPosition>(entity, position);
    auto& explosion = registry.Emplace<ComponentExplosion>(entity, sheet);
    explosion.m_sprite.SetClip("explode");
    explosion.m_sprite.SetPlaying(true);
    return entity;
}

void SystemExplosion::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    auto view = registry.View<ComponentExplosion>(moth::ecs::Exclude<DeadTag>);
    for (auto [entity, explosion] : view.each()) {
        explosion.m_sprite.Update(ticks);
        if (!explosion.m_sprite.IsPlaying()) {
            registry.Emplace<DeadTag>(entity);
        }
    }
}

void SystemExplosion::Draw(GameWorld& world, moth::gfx::IGraphics& graphics) {
    auto& registry = world.GetRegistry();

    auto cameraView = registry.View<ComponentCamera>();
    if (cameraView.empty()) {
        return;
    }

    auto& camera = cameraView.get<ComponentCamera>(*cameraView.begin());
    auto const cameraOffset = camera.m_offset + camera.m_shakeOffset;

    graphics.SetBlendMode(moth::gfx::BlendMode::Alpha);
    graphics.SetColor(moth::gfx::BasicColors::White);
    auto view = registry.View<ComponentExplosion, ComponentPosition>();
    for (auto [entity, explosion, pos] : view.each()) {
        auto const position = static_cast<moth::gfx::IntVec2>(pos.m_position + cameraOffset);
        moth::gfx::DrawSprite(graphics, explosion.m_sprite, position);
    }
}
