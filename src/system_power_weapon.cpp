#include "system_power_weapon.h"
#include "component_entity.h"
#include "game_world.h"
#include "gamedata_player.h"
#include "system_drawable.h"
#include "system_movement.h"
#include "system_projectile.h"
#include "system_shield.h"
#include "system_sound.h"
#include <moth/graphics/utils/math_utils.h>
#include <cmath>

namespace {
    // the release is a big moment, but smaller than the ship's death
    float const ReleaseShakeIntensity = 0.6f;
    uint32_t const ReleaseShakeDuration = 300;
    uint32_t const ReleaseFlashDuration = 150;
}

void SystemPowerWeapon::InitWeapon(moth::ecs::World& registry, moth::ecs::Entity entity,
                                   PowerWeaponData const& data) {
    if (!data.projectile.valid()) {
        spdlog::error("The power weapon has no valid projectile; the player has no power weapon");
        return;
    }
    auto& powerWeapon = registry.Emplace<ComponentPowerWeapon>(entity);
    powerWeapon.m_segmentPower = data.segment_power;
    powerWeapon.m_missilesPerSegment = data.missiles_per_segment;
    powerWeapon.m_damage = data.missile_damage;
    powerWeapon.m_projectile = *data.projectile;
    powerWeapon.m_muzzleSpeed = data.muzzle_speed;
}

void SystemPowerWeapon::Update(GameWorld& world, uint32_t ticks) {
    auto& registry = world.GetRegistry();
    bool released = false;
    bool empty = false;
    auto view = registry.View<ComponentPowerWeapon, ComponentPower, ComponentPosition, ComponentEntity>();
    for (auto [entity, weapon, power, position, details] : view.each()) {
        if (!weapon.m_activated) {
            continue;
        }
        weapon.m_activated = false;

        // every full segment is spent, and the partial one is kept
        auto const segments = weapon.m_segmentPower > 0.0f ? std::floor(power.m_power / weapon.m_segmentPower) : 0.0f;
        if (segments <= 0.0f) {
            empty = true;
            continue;
        }
        power.m_power -= segments * weapon.m_segmentPower;
        released = true;

        // the missiles fan out from the ship, at most 10 degrees apart, and each finds its own target
        auto const projectileCount = segments * static_cast<float>(weapon.m_missilesPerSegment);
        auto const maxSeparation = moth::gfx::DegToRad(10.0f);
        auto const anglePerProjectile = std::min(maxSeparation, moth::gfx::DegToRad(360.0f) / projectileCount);
        auto const fullSpread = anglePerProjectile * (projectileCount - 1);
        auto const startAngle = -fullSpread / 2.0f;

        for (uint32_t i = 0; i < static_cast<uint32_t>(projectileCount); ++i) {
            auto const angle = startAngle + (anglePerProjectile * static_cast<float>(i));
            auto const direction = moth::gfx::Rotate2D({ 0.0f, 1.0f }, angle);
            SystemProjectile::CreateProjectile(registry, weapon.m_projectile, entity, position.m_position,
                                               direction, weapon.m_muzzleSpeed, angle, weapon.m_damage);
        }
    }

    if (released) {
        SystemDrawable::StartScreenFlash(registry, ReleaseFlashDuration);
        SystemDrawable::StartCameraShake(registry, ReleaseShakeIntensity, ReleaseShakeDuration);
        SystemSound::Play(registry, "power_release");
    } else if (empty) {
        SystemSound::Play(registry, "power_empty");
    }
}
