#pragma once

#include <vector>

class GameWindow
{
    struct SDL_Window *window;
    struct SDL_Renderer *renderer;
    std::vector<class Actor*> actors;

    GameWindow(SDL_Window *window, SDL_Renderer *renderer);
public:
    static GameWindow *Create();
    ~GameWindow();

    struct SDL_Renderer *getRenderer() { return renderer;}

    void drawFrame();
};