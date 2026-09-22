#pragma once

#include "gamedata_entity.h"

// Lets an entity take part in body-to-body collisions. See ContactData for what the numbers mean.
struct ComponentContact {
    float m_damage = 0.0f;
    float m_selfDamage = 0.0f;
    int32_t m_cooldown = 0;
    // counts down after a contact. While it is running the entity neither deals nor receives contact
    // damage, so this doubles as the invulnerability window after being hit.
    int32_t m_cooldownLeft = 0;

    explicit ComponentContact(ContactData const& data)
        : m_damage(data.damage)
        , m_selfDamage(data.self_damage)
        , m_cooldown(data.cooldown) {}
};
