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

bool isSideWallTile(TileType tileType)
{
    switch (tileType)
    {
    case TileType::SideWall:
    case TileType::SideWallAngled1:
    case TileType::SideWallAngled2:
    case TileType::SideWallAngled3:
    case TileType::SideWallAngled4:
        return true;
    default:
        return false;
    }
}

bool isGroundTile(TileType tileType)
{
    switch (tileType)
    {
    case TileType::Ground:
    case TileType::GroundAngled1:
    case TileType::GroundAngled2:
    case TileType::GroundAngled3:
    case TileType::GroundAngled4:
        return true;
    default:
        return false;
    }
}

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

bool GameWindow::any_surface_intersects(const std::vector<SurfaceData> &surfaces, int mapX, int mapY)
{
    for(const auto& surface : surfaces){
        if(surface.mapRect.intersects(mapX, mapY)) {
            return true;
        }
    }
    return false;
}

tripoint GameWindow::getTripointAtMapPoint(int mapX, int mapY)
{
    for(const SurfaceData& groundSurface : groundSurfaces) {
        if(groundSurface.mapRect.intersects(mapX, mapY - 1)) {
            return {
                (mapX - groundSurface.mapRect.x1) + (mapY - groundSurface.mapRect.y1) / 2 + groundSurface.dimensions.x1, 
                groundSurface.dimensions.y1, 
                groundSurface.dimensions.z1 + (mapY - groundSurface.mapRect.y1)
            };
        }
    }
    for(const SurfaceData& wallSurface : wallSurfaces) {
        if(wallSurface.mapRect.intersects(mapX, mapY)) {
            if(wallSurface.dimensions.z2 > (wallSurface.dimensions.z1 + 1)) {
                return {
                    (mapX - wallSurface.mapRect.x1) / 2.f,
                    (float)mapY,
                    wallSurface.dimensions.z1 + (mapY - wallSurface.mapRect.y1) / 2.f
                };
            } else {
                return {
                    wallSurface.dimensions.x2 - wallSurface.dimensions.x1 + mapX, 
                    wallSurface.dimensions.y2 - wallSurface.dimensions.y1 + mapY,
                    wallSurface.dimensions.z2
                };
            }
        }
    }
    return {-1.f, -1.f, -1.f};
}

int GameWindow::getZLevelAtPoint(int mapX, int mapY)
{
    for(const SurfaceData& groundSurface : groundSurfaces) {
        if(groundSurface.mapRect.intersects(mapX, mapY - 1)) {
            return groundSurface.dimensions.z1 + (mapY - groundSurface.mapRect.y1);
        }
    }
    for(const SurfaceData& wallSurface : wallSurfaces) {
        if(wallSurface.mapRect.intersects(mapX, mapY - 1)) {
            if(wallSurface.dimensions.z2 > (wallSurface.dimensions.z1 + 1)) {
                return (mapX - wallSurface.mapRect.x1) / 2;
            } else {
                return  wallSurface.dimensions.z2;
            }
        } else if(wallSurface.mapRect.intersects(mapX + 1, mapY)) {
            //currentZ = wallSurface.dimensions.z2 - wallSurface.dimensions.z1 + 
        } else if (wallSurface.mapRect.intersects(mapX - 1, mapY)) {
            return wallSurface.dimensions.z1 + (wallSurface.mapRect.y2 - wallSurface.mapRect.y1);
        }
    }
    return -1;
}

GameWindow::GameWindow(SDL_Window *window, SDL_Renderer *renderer, tmx::Map& map, size_t sizex, size_t sizey) : 
    size_x(sizex),
    size_y(sizey),
    map(map),
    curTime(0),
    lastFrameTime(0)
{
    this->window = window;
    this->renderer = renderer;

    actors.push_back(new Actor(&sonicSpriteCfg, Texture::Create(renderer, "assets/images/sonic3.png"), 13, 11));

    //load the textures as they're shared between layers
    const auto& tileSets = map.getTilesets();
    assert(!tileSets.empty());
    for (const auto& ts : tileSets) {
        Texture* text = Texture::Create(renderer, ts.getImagePath());
        if (text) {
            textures.emplace_back(text);
        }
    }

    //load the layers
    const auto& mapLayers = map.getLayers();
    for (auto i = 0u; i < mapLayers.size(); ++i) {
        if (mapLayers[i]->getType() == tmx::Layer::Type::Tile) {
            renderLayers.emplace_back(std::make_unique<MapLayer>());
            renderLayers.back()->create(map, i, textures); //just cos we're using C++14
        }
    }
    readJsonTileData();
    auto bgLayer = getLayerByName("Background");
    auto mapSize = map.getTileCount();
    std::vector<SurfaceData> bgWallSurfaces, fgWallSurfaces, miscSurfaces;
    if(bgLayer != nullptr) {
        for (auto x = 0u; x < mapSize.x; ++x) {
            int currentZ = 0;
            for (auto y = 0u; y < mapSize.y; ++y) {
                TileType bgTileType = getTileType(x, y, mapSize.x, *bgLayer);
                if(bgTileType == TileType::Wall || bgTileType == TileType::SideWallAngled1) {
                    SurfaceData wallSurface;
                    if(bgTileType == TileType::Wall) {
                        traceWallTiles(x, y, mapSize, *bgLayer, currentZ, wallSurface);
                    } else {
                        traceSideWallTiles(x, y, mapSize, *bgLayer, currentZ, wallSurface);
                        currentZ = wallSurface.dimensions.z2;
                    }
                    bgWallSurfaces.push_back(wallSurface);
                    x = wallSurface.mapRect.x2;
                    break;
                }
            }
        }
    }
    auto fgWallLayer = getLayerByName("walls");
    if(fgWallLayer != nullptr) {
        for(auto x = 0u; x < mapSize.x; x++) {
            for(auto y = 0u; y < mapSize.y; y++) {
                TileType bgTileType = getTileType(x, y, mapSize.x, *fgWallLayer);
                if(bgTileType == TileType::Wall || bgTileType == TileType::SideWallAngled1) {
                    SurfaceData wallSurface;
                    if(bgTileType == TileType::Wall) {
                        traceWallTiles(x, y, mapSize, *fgWallLayer, 0, wallSurface);
                        for(const auto& fgWall: fgWallSurfaces) {
                            if(fgWall.mapRect.intersects(x - 1, y)) {
                                wallSurface.dimensions.z1 += fgWall.dimensions.z2;
                                wallSurface.dimensions.z2 += fgWall.dimensions.z2;
                                break;
                            }
                        }
                    } else {
                        traceSideWallTiles(x, y, mapSize, *fgWallLayer, 0, wallSurface);

                        for(const auto& bgWall: bgWallSurfaces) {
                            if(bgWall.mapRect.intersects(x - 1, y)) {
                                wallSurface.dimensions.z1 += bgWall.dimensions.z2;
                                wallSurface.dimensions.z2 += bgWall.dimensions.z2;
                                break;
                            }
                        }
                    }
                    fgWallSurfaces.push_back(wallSurface);
                    x = wallSurface.mapRect.x2;
                    break;
                }
            }
        }
    }
    wallSurfaces.insert(wallSurfaces.end(), bgWallSurfaces.begin(), bgWallSurfaces.end());
    wallSurfaces.insert(wallSurfaces.end(), fgWallSurfaces.begin(), fgWallSurfaces.end());
    auto fgLayer = getLayerByName("Foreground");
    if(fgLayer != nullptr) {
        for(auto x = 0u; x < mapSize.x; x++) {
            for(auto y = 0u; y < mapSize.y; y++) {
                TileType fgTileType = getTileType(x, y, mapSize.x, *fgLayer);
                if(isGroundTile(fgTileType) && !any_surface_intersects(groundSurfaces, x, y)) {
                    int currentZ = getZLevelAtPoint(x, y);
                    SurfaceData fgSurface;
                    traceGroundTiles(x, y, mapSize, *fgLayer, currentZ, fgSurface);
                    groundSurfaces.push_back(fgSurface);
                }
            }
        }
    }
    // convert to pixel dimensions
    // for(auto& sfc : wallSurfaces) {
    //     sfc.mapRect.x1 *= 16;
    //     sfc.mapRect.x2 *= 16;
    //     sfc.mapRect.y1 *= 16;
    //     sfc.mapRect.y2 *= 16;
    // }
    // for(auto& sfc : groundSurfaces) {
    //     sfc.mapRect.x1 *= 16;
    //     sfc.mapRect.x2 *= 16;
    //     sfc.mapRect.y1 *= 16;
    //     sfc.mapRect.y2 *= 16;
    // }
    z0_x = bgWallSurfaces[0].mapRect.x1;
    z0_y = bgWallSurfaces[0].mapRect.y1;
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

    for(int i = 0; i < len; i++) {
        fileData[i] = fgetc(jsonFile);
    }
    fclose(jsonFile);
    cJSON *json = cJSON_Parse(fileData);
    delete[] fileData;
    assert(json != nullptr && json->child != nullptr);

    cJSON *childJson = json->child;
    while (childJson != nullptr) {
        cJSON *tileData = childJson->child;
        TileType tileType = TileType::None;
        int id = atoi(childJson->string);
        while(tileData != nullptr) {
            char *str = tileData->valuestring;
            if(strcmp(tileData->string, "collisionPreset") == 0) {
                tileType = TileType::None;
                if(strcmp(str, "wall") == 0) {
                    tileType = TileType::Wall;
                } else if(strcmp(str, "sideWall") == 0) {
                    tileType = TileType::SideWall;
                } else if(strcmp(str, "sideWallAngled1") == 0) {
                    tileType = TileType::SideWallAngled1;
                } else if(strcmp(str, "sideWallAngled2") == 0) {
                    tileType = TileType::SideWallAngled2;
                } else if(strcmp(str, "sideWallAngled3") == 0) {
                    tileType = TileType::SideWallAngled3;
                } else if(strcmp(str, "sideWallAngled4") == 0) {
                    tileType = TileType::SideWallAngled4;
                } else if(strcmp(str, "box") == 0) {
                    tileType = TileType::Box;
                } else if(strcmp(str, "ground") == 0) {
                    tileType = TileType::Ground;
                } else if(strcmp(str, "groundAngled1") == 0) {
                    tileType = TileType::GroundAngled1;
                } else if(strcmp(str, "groundAngled2") == 0) {
                    tileType = TileType::GroundAngled2;
                } else if(strcmp(str, "groundAngled3") == 0) {
                    tileType = TileType::GroundAngled3;
                } else if(strcmp(str, "groundAngled4") == 0) {
                    tileType = TileType::GroundAngled4;
                }
            }
            tileData = tileData->next;
        }
        mapTileData[id] = tileType;
        childJson = childJson->next;
    }
    cJSON_Delete(json);
    return true;
}

TileType GameWindow::getTileType(int mapX, int mapY, int mapSizeX, const tmx::TileLayer &layer)
{
    int tileId = layer.getTiles()[mapSizeX * mapY + mapX].ID;
    if(tileId == 0) {
        return TileType::None;
    }
    int fgid = map.getTilesets()[0].getFirstGID();
    int lgid = map.getTilesets()[0].getLastGID();
    if(tileId >= fgid && tileId <= lgid) {
        tileId -= fgid;
    }
    TileType tileType = TileType::None;
    std::unordered_map<int, TileType>::iterator it = mapTileData.find(tileId);
    if(it != mapTileData.end()) {
        tileType = it->second;
    }
    return tileType;
}

tmx::TileLayer *GameWindow::getLayerByName(const char *name)
{
    const auto& layers = map.getLayers();
    for (auto i = 0u; i < layers.size(); ++i) {
        if(layers[i]->getType() == tmx::TileLayer::Type::Tile) {
            tmx::TileLayer &layer = layers[i]->getLayerAs<tmx::TileLayer>();
            if(layer.getName() == name) {
                return &layer;
            }
        }
    }
    return nullptr;
}

void GameWindow::traceGroundTiles(int mapX, int mapY, tmx::Vector2u& mapSize, tmx::TileLayer &layer, int currentZ, SurfaceData &surface)
{
    TileType lastTileType = TileType::None;
    TileType curTileType = getTileType(mapX, mapY, mapSize.x, layer);
    TileType expectedTileType = TileType::None;
    if(curTileType != TileType::GroundAngled1 && curTileType != TileType::GroundAngled2) {
        std::cout << "Bad map! Ground tiles organized in a way that tracer cannot trace the geometry!" << std::endl;
        return;
    }
    surface.dimensions.z1 = currentZ;
    surface.dimensions.z2 = currentZ + 1;
    surface.mapRect.x1 = mapX;
    surface.mapRect.x2 = mapX + 1;
    
    while(surface.mapRect.x2 < (layer.getSize().x - 1) && isGroundTile(getTileType(surface.mapRect.x2 + 1, mapY, mapSize.x, layer))) {
        surface.mapRect.x2++;
    }
    surface.dimensions.x1 = surface.mapRect.x1;
    surface.dimensions.x2 = surface.mapRect.x2 + 1;
    surface.dimensions.y1 = surface.dimensions.y2 = mapY;
    surface.dimensions.y2++;
    surface.mapRect.y1 = surface.mapRect.y2 = mapY;
    int tmpX1 = surface.mapRect.x1;
    int tmpX2 = surface.mapRect.x2;
    TileType leftTile = getTileType(tmpX1, surface.mapRect.y2, mapSize.x, layer);
    TileType rightTile = getTileType(tmpX2, surface.mapRect.y2, mapSize.x, layer);
    while(surface.mapRect.y2 < layer.getSize().y) {
        if(leftTile == TileType::GroundAngled2) {
            // as we move down the pattern of ground tiles, adjust X to match the angle of the tiles
            tmpX1++;
        }
        if(rightTile == TileType::GroundAngled4) {
            // as we move down the pattern of ground tiles, adjust X to match the angle of the tiles
            tmpX2++;
        }
        leftTile = getTileType(tmpX1, surface.mapRect.y2 + 1, mapSize.x, layer);
        rightTile = getTileType(tmpX2, surface.mapRect.y2 + 1, mapSize.x, layer);
        if(!((leftTile == TileType::GroundAngled1 || leftTile == TileType::GroundAngled2) || 
            (rightTile == TileType::GroundAngled3 || rightTile == TileType::GroundAngled4))) {
            break;
        }
        surface.mapRect.x2 = tmpX2;
        surface.mapRect.y2++;
        surface.dimensions.z2++;
    }
    if(surface.mapRect.y2 == mapY) {
        std::cout << "Bad map! Did not parse a single row of ground tiles!" << std::endl;
    }
}

void GameWindow::traceWallTiles(int mapX, int mapY, tmx::Vector2u& mapSize, tmx::TileLayer &layer, int currentZ, SurfaceData &surface)
{
    TileType lastTileType = TileType::None;
    TileType curTileType = getTileType(mapX, mapY, mapSize.x, layer);
    TileType expectedTileType = TileType::None;
    if(curTileType != TileType::Wall && curTileType != TileType::SideWallAngled1) {
        std::cout << "Bad map! Wall tiles organized in a way that tracer cannot trace the geometry!" << std::endl;
        return;
    }
    surface.mapRect.y1 = surface.mapRect.y2 = mapY;
    while((surface.mapRect.y2 + 1) < mapSize.y && getTileType(mapX, surface.mapRect.y2 + 1, mapSize.x, layer) == TileType::Wall) {
        surface.mapRect.y2++;
    }
    surface.mapRect.x1 = mapX;
    surface.mapRect.x2 = mapX + 1;
    while((surface.mapRect.x2 + 1) < mapSize.x && 
        getTileType(surface.mapRect.x2 + 1, surface.mapRect.y1, mapSize.x, layer) == TileType::Wall && 
        getTileType(surface.mapRect.x2 + 1, surface.mapRect.y2, mapSize.x, layer) == TileType::Wall) {

        surface.mapRect.x2++;
    }
    surface.dimensions.z1 = surface.dimensions.z2 = currentZ;
    surface.dimensions.z2++;
    surface.dimensions.x1 = surface.mapRect.x1;
    surface.dimensions.x2 = surface.mapRect.x2;
    surface.dimensions.y1 = surface.mapRect.y1;
    surface.dimensions.y2 = surface.mapRect.y2;
}

/// @brief Trace a 3D ground surface from the 2D map by looking at the geometry of the tiles as defined in JSON
/// @param mapX X-coordinate on the 2D map
/// @param mapY Y-coordinate on the 2D map
/// @param layer Current layer being considered
/// @param currentZ derived Z-point in 3D space
/// @param surface Surface data to be written, containing the 3D collision data from the detected surface
void GameWindow::traceSideWallTiles(int mapX, int mapY, tmx::Vector2u& mapSize, tmx::TileLayer &layer, int currentZ, SurfaceData &surface)
{
    TileType lastTileType = TileType::None;
    TileType curTileType = getTileType(mapX, mapY, mapSize.x, layer);
    TileType expectedTileType = TileType::None;
    if(curTileType != TileType::SideWallAngled1 && curTileType != TileType::SideWallAngled2) {
        std::cout << "Bad map! Side-wall tiles organized in a way that tracer cannot trace the geometry!" << std::endl;
        return;
    }
    surface.mapRect.y1 = surface.mapRect.y2 = mapY;
    while(surface.mapRect.y2 + 1 < mapSize.y && isSideWallTile(getTileType(mapX, surface.mapRect.y2 + 1, mapSize.x, layer))) {
        surface.mapRect.y2++;
    }
    surface.dimensions.y1 = mapY;
    // We set y2 here, because as we "descend" down the angle of the wall in 2D space, the 3D y-value does not change
    surface.dimensions.y2 = surface.mapRect.y2;
    surface.dimensions.z1 = surface.dimensions.z2 = currentZ;
    surface.mapRect.x1 = mapX;
    surface.mapRect.x2 = mapX;
    surface.dimensions.x1 = surface.mapRect.x1;
    int ylen = surface.mapRect.y2 - surface.mapRect.y1;
    while(surface.mapRect.x2 < layer.getSize().x && 
        surface.mapRect.y2 < layer.getSize().y) {
        TileType leftTile = getTileType(surface.mapRect.x2, surface.mapRect.y2 - ylen, mapSize.x, layer);
        TileType rightTile = getTileType(surface.mapRect.x2, surface.mapRect.y2, mapSize.x, layer);
        if(!((leftTile == TileType::SideWallAngled1 && rightTile == TileType::SideWallAngled4) || 
            (leftTile == TileType::SideWallAngled2 && rightTile == TileType::SideWallAngled3))) {
            break;
        }
        surface.mapRect.y2 += 2;
        surface.mapRect.x2++;
        surface.dimensions.z2 += 2;
    }
    surface.dimensions.x2 = surface.mapRect.x2;
    if(surface.mapRect.y2 == mapY) {
        std::cout << "Bad map! Did not parse a single column of side-wall tiles!" << std::endl;
    }
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
    if (!map.load("assets/robotropolis.tmx")) {
        SDL_Log("Failed to load map: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return nullptr;
    }
    return new GameWindow(window, renderer, map, 852, 480);
}

void GameWindow::handle_input(const SDL_Event& event)
{
    for (Actor* actor : actors) {
        actor->handle_input(event);
    }
}

void GameWindow::drawFrame()
{
    curTime = SDL_GetTicks64();
    uint64_t frameDeltaTime = lastFrameTime - curTime;
    SDL_RenderClear(renderer);
    for (const auto& l : renderLayers) {
        l->draw(renderer);
    }
    for (Actor* actor : actors) {
        actor->draw(this, frameDeltaTime);
    }
    SDL_RenderPresent(renderer);
    lastFrameTime = curTime;
}