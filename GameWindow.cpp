#include "GameWindow.h"
#include "SpriteSheet.h"
#include "Actor.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "MapLayer.h"
#include <tmxlite/Map.hpp>

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

GameWindow::GameWindow(SDL_Window *window, SDL_Renderer *renderer, tmx::Map& map, size_t sizex, size_t sizey) : 
    size_x(sizex),
    size_y(sizey),
    map(map),
    curTime(0),
    lastFrameTime(0)
{
    this->window = window;
    this->renderer = renderer;

    actors.push_back(new Actor(&sonicSpriteCfg, Texture::Create(renderer, "sprites/sonic3.png"), 50, 0, 50));

    //load the textures as they're shared between layers
    const auto& tileSets = map.getTilesets();
    assert(!tileSets.empty());
    for (const auto& ts : tileSets)
    {
        Texture* text = Texture::Create(renderer, ts.getImagePath());
        if (text)
        {
            textures.emplace_back(text);
        }
    }

    //load the layers
    const auto& mapLayers = map.getLayers();
    for (auto i = 0u; i < mapLayers.size(); ++i)
    {
        if (mapLayers[i]->getType() == tmx::Layer::Type::Tile)
        {
            renderLayers.emplace_back(std::make_unique<MapLayer>());
            renderLayers.back()->create(map, i, textures); //just cos we're using C++14
        }
    }
    
}

GameWindow::~GameWindow()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

int GameWindow::getHeight(int x, int y)
{
    size_t tile_x = x / 16;
    size_t tile_y = y / 16;
    size_t idx = (x / 16) * map.getTileSize().x + (y / 16);
    map.getTilesets()[0].getTiles()[idx];
}

GameWindow *GameWindow::Create()
{
    if(SDL_Init( SDL_INIT_VIDEO ) < 0) return nullptr;
  
    
    SDL_Window *window = SDL_CreateWindow("Sonic Freedom Fighters", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
  
    if(window == nullptr) 
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        return nullptr;
    }
  
    SDL_Renderer *renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED |
                                        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE );
    if(renderer == nullptr)
    {
        SDL_DestroyWindow(window);
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        SDL_Quit();
        return nullptr;
    }
    if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) 
    {
        SDL_Log("Failed to initialize SDL_Image: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return nullptr;
    }

    std::vector<std::unique_ptr<Texture>> textures;
    std::vector<std::unique_ptr<MapLayer>> renderLayers;
    tmx::Map map;
    if (!map.load("assets/demo.tmx"))
    {
        SDL_Log("Failed to load map: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return nullptr;
    }
    return new GameWindow(window, renderer, map, 1280, 720);
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
    curTime = SDL_GetTicks64();
    uint64_t frameDeltaTime = lastFrameTime - curTime;
    SDL_RenderClear(renderer);
    for (const auto& l : renderLayers)
    {
        l->draw(renderer);
    }
    for (Actor* actor : actors)
    {
        actor->draw(this, frameDeltaTime);
    }
    SDL_RenderPresent(renderer);
    lastFrameTime = curTime;
}