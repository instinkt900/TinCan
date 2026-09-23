#pragma once

#include "game_world.h"

class SystemPlayerEditor {
public:
    // Unlike the behaviour and formation editors this one draws nothing in the world view, so it never
    // takes ownership of it and the active editor window is left alone.
    static void Draw(GameWorld& world);
};
