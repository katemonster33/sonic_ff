#include "GameWindow.h"
#include "SpriteSheet.h"
#include "Actor.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "MapLayer.h"
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <cjson/cJSON.h>
#include <iostream>

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

    actors.push_back(new Actor(&sonicSpriteCfg, Texture::Create(renderer, "assets/images/sonic3.png"), 50, 0, 50));

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

int getFileLength(FILE *file)
{
    //Find the length
    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    rewind(file);
    return length;
}

bool GameWindow::readJsonTileData()
{
    FILE *jsonFile = fopen("assets/robotropolis-sheet.json", "r");
    assert(jsonFile != nullptr);

    int len = getFileLength(jsonFile);
    char *fileData = new char[len];

    for(int i = 0; i < len; i++)
    {
        fileData[i] = fgetc(jsonFile);
    }
    fclose(jsonFile);
    cJSON *json = cJSON_Parse(fileData);
    delete fileData;
    assert(json != nullptr && json->child != nullptr);

    cJSON *childJson = json->child;
    while (childJson != nullptr) {
        cJSON *tileData = childJson->child;
        TileData data;
        data.id = atoi(childJson->string);
        while(tileData != nullptr) {
            char *str = tileData->valuestring;
            if(strcmp(tileData->string, "collisionPreset") == 0) {
                data.tileType = TileType::None;
                if(strcmp(str, "wall") == 0) {
                    data.tileType = TileType::Wall;
                } else if(strcmp(str, "sideWall") == 0) {
                    data.tileType = TileType::SideWall;
                } else if(strcmp(str, "sideWallAngled1") == 0) {
                    data.tileType = TileType::SideWallAngled1;
                } else if(strcmp(str, "sideWallAngled2") == 0) {
                    data.tileType = TileType::SideWallAngled2;
                } else if(strcmp(str, "sideWallAngled3") == 0) {
                    data.tileType = TileType::SideWallAngled3;
                } else if(strcmp(str, "sideWallAngled4") == 0) {
                    data.tileType = TileType::SideWallAngled4;
                } else if(strcmp(str, "box") == 0) {
                    data.tileType = TileType::Box;
                } else if(strcmp(str, "ground") == 0) {
                    data.tileType = TileType::Ground;
                } else if(strcmp(str, "groundAngled1") == 0) {
                    data.tileType = TileType::GroundAngled1;
                } else if(strcmp(str, "groundAngled2") == 0) {
                    data.tileType = TileType::GroundAngled2;
                } else if(strcmp(str, "groundAngled3") == 0) {
                    data.tileType = TileType::GroundAngled3;
                } else if(strcmp(str, "groundAngled4") == 0) {
                    data.tileType = TileType::GroundAngled4;
                }
            }
            tileData = tileData->next;
        }
        mapTileData[data.id] = data;
        childJson = childJson->next;
    }
    cJSON_Delete(json);
    return true;
}

TileType GameWindow::getTileType(int mapX, int mapY, int mapSizeX, const tmx::TileLayer &layer)
{
    int tileId = layer.getTiles()[mapSizeX * mapY + mapX].ID;
    TileType tileType = TileType::None;
    std::unordered_map<int, TileData>::iterator it = mapTileData.find(tileId);
    if(it != mapTileData.end())
    {
        tileType = it->second.tileType;
    }
    return tileType;
}

void GameWindow::traceGroundTiles(int mapX, int mapY, int mapSizeX, tmx::TileLayer &layer, int currentZ, SurfaceData &surface)
{
    TileType lastTileType = TileType::None;
    SurfaceData *output = new SurfaceData;
    TileType curTileType = getTileType(mapX, mapY, mapSizeX, layer);
    TileType expectedTileType = TileType::None;
    if(curTileType != TileType::GroundAngled1 && curTileType != TileType::GroundAngled2)
    {
        std::cout << "Bad map! Ground tiles organized in a way that tracer cannot trace the geometry!" << std::endl;
        return;
    }
    surface.dimensions.z1 = currentZ;
    surface.dimensions.z2 = currentZ + 1;
    surface.mapRect.x1 = mapX;
    surface.mapRect.x2 = mapX + 1;
    while(getTileType(surface.mapRect.x2, mapY, mapSizeX, layer) == TileType::Ground)
    {
        surface.mapRect.x2++;
    }
    surface.mapRect.y1 = mapY;
    surface.mapRect.y2 = mapY;
    int tmpX = surface.mapRect.x1;
    int xlen = surface.mapRect.x2 - surface.mapRect.x1;
    while(surface.mapRect.y2 < layer.getSize().y)
    {
        TileType leftTile = getTileType(tmpX, surface.mapRect.y2 + 1, mapSizeX, layer);
        TileType rightTile = getTileType(tmpX + xlen, surface.mapRect.y2 + 1, mapSizeX, layer);
        if(!((leftTile == TileType::GroundAngled1 && rightTile == TileType::GroundAngled3) || 
            (leftTile == TileType::GroundAngled2 && rightTile == TileType::GroundAngled4))) {
            break;
        }
        if(leftTile == TileType::GroundAngled2) {
            // as we move down the pattern of ground tiles, adjust X to match the angle of the tiles
            tmpX++;
        }
        surface.mapRect.y2++;
    }
    if(surface.mapRect.y2 == mapY)
    {
        std::cout << "Bad map! Did not parse a single row of ground tiles!" << std::endl;
    }
}

void GameWindow::traceWallTiles(int mapX, int mapY, int mapSizeX, tmx::TileLayer &layer, int currentZ, SurfaceData &surface)
{
    TileType lastTileType = TileType::None;
    SurfaceData *output = new SurfaceData;
    TileType curTileType = getTileType(mapX, mapY, mapSizeX, layer);
    TileType expectedTileType = TileType::None;
    if(curTileType != TileType::Wall && curTileType != TileType::SideWallAngled1)
    {
        std::cout << "Bad map! Ground tiles organized in a way that tracer cannot trace the geometry!" << std::endl;
        return;
    }
    bool sideWallMode = curTileType == TileType::SideWallAngled1;
    surface.dimensions.z1 = currentZ;
    surface.dimensions.z2 = currentZ + 1;
    surface.mapRect.x1 = mapX;
    surface.mapRect.x2 = mapX + 1;
    while(getTileType(surface.mapRect.x2, mapY, mapSizeX, layer) == TileType::Ground)
    {
        surface.mapRect.x2++;
    }
    surface.mapRect.y1 = mapY;
    surface.mapRect.y2 = mapY;
    int tmpX = surface.mapRect.x1;
    int xlen = surface.mapRect.x2 - surface.mapRect.x1;
    while(surface.mapRect.y2 < layer.getSize().y)
    {
        TileType leftTile = getTileType(tmpX, surface.mapRect.y2 + 1, mapSizeX, layer);
        TileType rightTile = getTileType(tmpX + xlen, surface.mapRect.y2 + 1, mapSizeX, layer);
        if(!((leftTile == TileType::GroundAngled1 && rightTile == TileType::GroundAngled3) || 
            (leftTile == TileType::GroundAngled2 && rightTile == TileType::GroundAngled4))) {
            break;
        }
        if(leftTile == TileType::GroundAngled2) {
            // as we move down the pattern of ground tiles, adjust X to match the angle of the tiles
            tmpX++;
        }
        surface.mapRect.y2++;
    }
    if(surface.mapRect.y2 == mapY)
    {
        std::cout << "Bad map! Did not parse a single row of ground tiles!" << std::endl;
    }
}

void GameWindow::traceSideWallTiles(int mapX, int mapY, int mapSizeX, tmx::TileLayer &layer, int currentZ, SurfaceData &surface)
{
    TileType lastTileType = TileType::None;
    SurfaceData *output = new SurfaceData;
    TileType curTileType = getTileType(mapX, mapY, mapSizeX, layer);
    TileType expectedTileType = TileType::None;
    if(curTileType != TileType::SideWallAngled1 && curTileType != TileType::SideWallAngled2)
    {
        std::cout << "Bad map! Ground tiles organized in a way that tracer cannot trace the geometry!" << std::endl;
        return;
    }
    bool sideWallMode = curTileType == TileType::SideWallAngled1;
    surface.dimensions.z1 = currentZ;
    surface.dimensions.z2 = currentZ + 1;
    surface.mapRect.x1 = mapX;
    surface.mapRect.x2 = mapX + 1;
    while(getTileType(surface.mapRect.x2, mapY, mapSizeX, layer) == TileType::Ground)
    {
        surface.mapRect.x2++;
    }
    surface.mapRect.y1 = mapY;
    surface.mapRect.y2 = mapY;
    int tmpX = surface.mapRect.x1;
    int xlen = surface.mapRect.x2 - surface.mapRect.x1;
    while(surface.mapRect.y2 < layer.getSize().y)
    {
        TileType leftTile = getTileType(tmpX, surface.mapRect.y2 + 1, mapSizeX, layer);
        TileType rightTile = getTileType(tmpX + xlen, surface.mapRect.y2 + 1, mapSizeX, layer);
        if(!((leftTile == TileType::GroundAngled1 && rightTile == TileType::GroundAngled3) || 
            (leftTile == TileType::GroundAngled2 && rightTile == TileType::GroundAngled4))) {
            break;
        }
        if(leftTile == TileType::GroundAngled2) {
            // as we move down the pattern of ground tiles, adjust X to match the angle of the tiles
            tmpX++;
        }
        surface.mapRect.y2++;
    }
    if(surface.mapRect.y2 == mapY)
    {
        std::cout << "Bad map! Did not parse a single row of ground tiles!" << std::endl;
    }
}

/// @brief Trace a 3D surface from the 2D map by looking at the geometry of the tiles as defined in JSON
/// @param mapX X-coordinate on the 2D map
/// @param mapY Y-coordinate on the 2D map
/// @param currentZ derived Z-point in 3D space from 
/// @return Surface data containing the 3D collision data from the detected surface
SurfaceData *GameWindow::createSurfaceFromMap(int mapX, int mapY, int mapSizeX, tmx::TileLayer &layer, int currentZ)
{

    int tileId = layer.getTiles()[mapSizeX * mapY + mapX].ID;

    TileType tileType = TileType::None;
    std::unordered_map<int, TileData>::iterator it = mapTileData.find(tileId);
    if(it != mapTileData.end())
    {
        tileType = it->second.tileType;
    }
    if(tileType == TileType::None)
    {
        return nullptr;
    }
    SurfaceData *output = new SurfaceData;
    switch(tileType)
    {
        case TileType::Ground:
        case TileType::GroundAngled1:
        case TileType::GroundAngled2:
        case TileType::GroundAngled3:
        case TileType::GroundAngled4:
        break;
        case TileType::Box:
        break;
        case TileType::Wall:
        break;
        case TileType::SideWall:
        case TileType::SideWallAngled1:
        case TileType::SideWallAngled2:
        case TileType::SideWallAngled3:
        case TileType::SideWallAngled4:
        break;
    }
    return output;
}

int GameWindow::getHeight(int x, int y)
{
    return 1;
    //size_t tile_x = x / 16;
    //size_t tile_y = y / 16;
    //size_t idx = (x / 16) * map.getTileSize().x + (y / 16);
    //map.getTilesets()[0].getTiles()[idx];
}

GameWindow *GameWindow::Create()
{
    if(SDL_Init( SDL_INIT_VIDEO ) < 0) return nullptr;
  
    
    SDL_Window *window = SDL_CreateWindow("Sonic Freedom Fighters", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 852, 480, SDL_WINDOW_SHOWN);
  
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
    if (!map.load("assets/robotropolis.tmx"))
    {
        SDL_Log("Failed to load map: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return nullptr;
    }
    return new GameWindow(window, renderer, map, 852, 480);
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