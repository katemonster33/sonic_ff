#pragma once

#include <vector>
#include "Actor.h"

class GameWindow
{
    std::vector<Actor*> actors;

public:
    GameWindow()
    {
    }

    void drawFrame();
}