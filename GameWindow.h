#pragma once

#include <vector>

class GameWindow
{
    struct SDL_Window *window;
    struct SDL_Renderer *renderer;
    size_t size_x;
    size_t size_y;
    std::vector<class Actor*> actors;
    std::vector<class Actor*> actors;

    GameWindow(SDL_Window *window, SDL_Renderer *renderer, size_t sizex, size_t sizey);
public:
    static GameWindow *Create();
    ~GameWindow();

    struct SDL_Renderer *getRenderer() { return renderer;}
    size_t GetSizeX() { return size_x; }
    size_t GetSizeY() { return size_y; }

    void handle_input(const union SDL_Event& event);
    void drawFrame();
};