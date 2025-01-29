#pragma once

#include <vector>
#include <memory>

namespace tmx
{
    class Map;
}

class GameWindow
{
    struct SDL_Window *window;
    struct SDL_Renderer *renderer;
    std::vector<std::unique_ptr<class MapLayer>> renderLayers;
    std::vector<std::unique_ptr<class Texture>> textures;
    tmx::Map& map;
    size_t size_x;
    size_t size_y;
    std::vector<class Actor*> actors;
    uint64_t curTime;
    uint64_t lastFrameTime;

    GameWindow(SDL_Window *window, SDL_Renderer *renderer, tmx::Map& map, size_t sizex, size_t sizey);
public:
    static GameWindow *Create();
    ~GameWindow();

    struct SDL_Renderer *getRenderer() { return renderer;}
    size_t GetSizeX() { return size_x; }
    size_t GetSizeY() { return size_y; }

    void handle_input(const union SDL_Event& event);
    int getHeight(int x, int y);
    void drawFrame();
};