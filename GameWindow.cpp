#include "GameWindow.h"
#include "SpriteSheet.h"
#include "Actor.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


SpriteConfig sonicSpriteCfg{
  "Sonic",
  {
    {
      ActorState::Default,
      {
        { 0, 0, 30, 42 }
      }
    },
    {
      ActorState::Idle,
      {
        { 30, 0, 31, 42 },
        { 61, 0, 32, 42 },
        { 94, 0, 33, 42 },
        { 127, 0, 33, 42 },
        { 160, 0, 33, 42 },
        { 193, 0, 33, 42 },
        { 226, 0, 34, 42 },
        { 260, 0, 37, 42 },
        { 297, 0, 34, 42 },
        { 330, 0, 33, 42 },
        { 363, 0, 28, 42 },
        { 391, 0, 28, 42 }
      }
    },
    {
      ActorState::LookingUp,
      {
        { 419, 0, 33, 42 }, { 452, 0, 28, 42 }
      }
    }
  }
};

GameWindow::GameWindow(SDL_Window *window, SDL_Renderer *renderer, size_t sizex, size_t sizey) : 
    size_x(sizex),
    size_y(sizey)
{
    this->window = window;
    this->renderer = renderer;

    actors.push_back(new Actor(&sonicSpriteCfg, Texture::Create(this, "sprites/sonic3.png"), 50, 50));
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
    return new GameWindow(window, renderer, 1280, 720);
}

void GameWindow::handle_input(const SDL_Event& event)
{
    for (Actor* actor : actors)
    {
        actor->handle_input(event);
    }
}

void GameWindow::drawFrame()
{
    SDL_RenderClear(renderer);
    for (Actor* actor : actors)
    {
        actor->draw(this);
    }
    SDL_RenderPresent(renderer);
}