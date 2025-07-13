#pragma once

#include "gamedata_sprite.h"
#include <canyon/graphics/igraphics.h>

class GameWorld;

struct ComponentDrawable {
    SpriteData m_spriteData;
};

class SystemDrawable {
public:
    static void Update(GameWorld& world, canyon::graphics::IGraphics& graphics);
};
