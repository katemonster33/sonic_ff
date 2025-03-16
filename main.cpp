#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "GameWindow.h"
#include "Texture.h"
#include "MapLayer.h"
#include <tmxlite/Map.hpp>
//#include "SimpleJSON/json.hpp"

int main(int argc, char** argv) {
    GameWindow* gameWindow = GameWindow::Create();

    if (!gameWindow)
    {
        SDL_Quit();
        std::cout << "SDL init failed." << std::endl;
        return -1;
    }
    bool game_open = true;
    SDL_Event event;
    while (game_open)
    {
        while (SDL_PollEvent(&event) > 0)
        {
            if (event.type == SDL_QUIT) {
                delete gameWindow;
                return 0;
            }
            gameWindow->handle_input(event);
        }
        gameWindow->drawFrame();
        SDL_Delay(10);
    }
    delete gameWindow;
    return 0;
}