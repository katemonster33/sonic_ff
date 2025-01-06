#pragma once

#include <vector>
#include "Actor.h"

class GameWindow
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    std::vector<Actor*> actors;

    GameWindow(SDL_Window *window, SDL_Renderer *renderer);
public:
    static GameWindow *Create();
    ~GameWindow();

    SDL_Renderer *getRenderer() { return renderer;}

    void drawFrame();
};