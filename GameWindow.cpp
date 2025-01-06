#include "GameWindow.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>>

GameWindow::GameWindow(SDL_Window *window, SDL_Renderer *renderer)
{
    this->window = window;
    this->renderer = renderer;
}

GameWindow::~GameWindow()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

GameWindow *GameWindow::Create()
{

    if(SDL_Init( SDL_INIT_VIDEO ) < 0) return nullptr;
  
    SDL_Window *window = SDL_CreateWindow("Sonic Freedom Fighters", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
  
    if(window == NULL) 
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return nullptr;
    }
  
    SDL_Renderer *renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED |
                                        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE );
    if(renderer == nullptr)
    {
        SDL_DestroyWindow(window);
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return nullptr;
    }
    if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) 
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return nullptr;
    }
    return new GameWindow(window, renderer);
}

void GameWindow::drawFrame()
{
    
}