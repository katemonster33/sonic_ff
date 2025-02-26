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
#include <fstream>

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

bool GameWindow::any_surface_intersects(const std::vector<SurfaceData> &surfaces, const mappoint& mt)
{
    for(const auto& surface : surfaces) {
        if(surface.mapRect.intersects(mt)) {
            return true;
        }
    }
    return false;
}

tripoint GameWindow::getTripointAtMapPoint(const mappoint& mt)
{
    int zlevel = getZLevelAtPoint(mt);
    tripoint output{ -1.f, -1.f, -1.f };
    if (zlevel != -1) {
        getRealPosFromMapPos(mt, output, zlevel);
    }
    return output;
}

int GameWindow::getZLevelAtPoint(const mappoint& mt, TileLayerId layer)
{
    if (layer == TileLayerId::Ground || layer == TileLayerId::Any) {
        for (const SurfaceData& groundSurface : surfaces) {
            if (groundSurface.layer == TileLayerId::Ground && groundSurface.mapRect.intersects(mappoint{ mt.x, mt.y - 1 })) {
                return groundSurface.dimensions.p2.z;
            }
        }
    }
    for(const auto &surface : surfaces) {
        if (surface.layer == layer || layer == TileLayerId::Any) {
            if (surface.mapRect.intersects(mappoint{ mt.x, mt.y - 1 })) {
                if (surface.dimensions.p2.z > (surface.dimensions.p1.z + 1)) {
                    return (mt.x - surface.mapRect.p1.x) / 2;
                } else {
                    return  surface.dimensions.p2.z;
                }
            } else if (surface.mapRect.intersects(mappoint{ mt.x + 1, mt.y })) {
                //currentZ = surface.dimensions.p2.z - surface.dimensions.p1.z + 
            } /*else if (surface.mapRect.intersects(mappoint{ mt.x - 1, mt.y })) {
                return surface.dimensions.p1.z + (surface.mapRect.p2.y - surface.mapRect.p1.y);
            }*/
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

    actors.push_back(new Actor(&sonicSpriteCfg, Texture::Create(renderer, "assets/images/sonic3.png"), { 13, 11 }));

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
    std::vector<SurfaceData> miscSurfaces;
    if(bgLayer != nullptr) {
        int currentZ = 0;
        mappoint mt{ 0, 0 };
        for (; mt.x < mapSize.x; ++mt.x) {
            for (mt.y = 0; mt.y < mapSize.y; ++mt.y) {
                TileType bgTileType = getTileType(mt, mapSize.x, *bgLayer);
                if(bgTileType == TileType::Wall || bgTileType == TileType::SideWallAngled1) {
                    SurfaceData wallSurface;
                    wallSurface.layer = TileLayerId::BackgroundWall;
                    bool traceSuccess = false;
                    if(bgTileType == TileType::Wall) {
                        traceSuccess = traceWallTiles(mt, mapSize, *bgLayer, currentZ, wallSurface);
                    } else {
                        traceSuccess = traceSideWallTiles(mt, mapSize, *bgLayer, currentZ, wallSurface);
                        currentZ = wallSurface.dimensions.p2.z;
                    }
                    if (traceSuccess) {
                        surfaces.push_back(wallSurface);
                        mt.x = wallSurface.mapRect.p2.x - 1;
                        break;
                    }
                }
            }
        }
    }
    auto fgWallLayer = getLayerByName("walls");
    if(fgWallLayer != nullptr) {
        mappoint mt{ 0, 0 };
        for(; mt.x < mapSize.x; mt.x++) {
            for(mt.y = 0; mt.y < mapSize.y; mt.y++) {
                TileType bgTileType = getTileType(mt, mapSize.x, *fgWallLayer);
                if(bgTileType == TileType::Wall || bgTileType == TileType::SideWallAngled1) {
                    SurfaceData wallSurface;
                    wallSurface.layer = TileLayerId::ForegroundWall;
                    bool parseSuccess = false;
                    int zOffset = 0;
                    if(bgTileType == TileType::Wall) {
                        parseSuccess = traceWallTiles(mt, mapSize, *fgWallLayer, 0, wallSurface);
                        zOffset = getZLevelAtPoint({ mt.x, mt.y }, TileLayerId::ForegroundWall);
                    } else {
                        parseSuccess = traceSideWallTiles(mt, mapSize, *fgWallLayer, 0, wallSurface);
                        zOffset = getZLevelAtPoint({ mt.x - 1, mt.y }, TileLayerId::BackgroundWall);
                    }
                    if (parseSuccess) {
                        wallSurface.dimensions.p1.z += zOffset;
                        wallSurface.dimensions.p2.z += zOffset;
                        surfaces.push_back(wallSurface);
                        mt.x = wallSurface.mapRect.p2.x;
                        break;
                    }
                }
            }
        }
    }
    auto fgLayer = getLayerByName("Foreground");
    if(fgLayer != nullptr) {
        mappoint mt{ 0, 0 };
        for(; mt.x < mapSize.x; mt.x++) {
            for(mt.y = 0; mt.y < mapSize.y; mt.y++) {
                TileType fgTileType = getTileType(mt, mapSize.x, *fgLayer);
                if(isGroundTile(fgTileType) && !any_surface_intersects(get_ground_geometries(), mt)) {
                    int currentZ = getZLevelAtPoint(mt);
                    SurfaceData fgSurface;
                    fgSurface.layer = TileLayerId::Ground;
                    traceGroundTiles(mt, mapSize, *fgLayer, currentZ, fgSurface);
                    surfaces.push_back(fgSurface);
                }
            }
        }
    }
    z0_x = surfaces[0].mapRect.p1.x;
    z0_y = surfaces[0].mapRect.p1.y;
}

GameWindow::~GameWindow()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

int getFileLength(std::ifstream &file)
{
    file.seekg(0, std::ios::end);
    size_t length = file.tellg();
    file.seekg(0, std::ios::beg);
    return length;
}

bool GameWindow::readJsonTileData()
{
    std::ifstream jsonFile("assets/robotropolis-sheet.json", std::ios::in);
    assert(jsonFile.is_open());

    int len = getFileLength(jsonFile);
    char *fileData = new char[len];
    jsonFile.read(fileData, len);
    jsonFile.close();

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

TileType GameWindow::getTileType(const mappoint &mt, int mapSizeX, const tmx::TileLayer &layer)
{
    int tileId = layer.getTiles()[mapSizeX * mt.y + mt.x].ID;
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

bool GameWindow::getNextSideGroundTile(mappoint& mt, unsigned int mapSizeX, tmx::TileLayer& layer)
{
    TileType tileTmp = getTileType(mt, mapSizeX, layer);
    if (tileTmp == TileType::GroundAngled2 || tileTmp == TileType::GroundAngled4) {
        mt.x++;
        mt.y++;
        return true;
    } else if (tileTmp == TileType::GroundAngled1 ||  tileTmp == TileType::GroundAngled3) {
        mt.y++;
        return true;
    } else {
        return false;
    }
}

bool GameWindow::traceGroundTiles(const mappoint& mt, tmx::Vector2u& mapSize, tmx::TileLayer &layer, int currentZ, SurfaceData &surface)
{
    TileType lastTileType = TileType::None;
    TileType curTileType = getTileType(mt, mapSize.x, layer);
    TileType expectedTileType = TileType::None;
    if(curTileType != TileType::GroundAngled1 && curTileType != TileType::GroundAngled2) {
        std::cout << "Bad map! Ground tiles organized in a way that tracer cannot trace the geometry!" << std::endl;
        return false;
    }
    surface.mapRect.p1 = surface.mapRect.p2 = mt;
    
    while(surface.mapRect.p2.x < mapSize.x && isGroundTile(getTileType(surface.mapRect.p2, mapSize.x, layer))) {
        surface.mapRect.p2.x++;
    }
    mappoint leftTile = mt;
    mappoint rightTile{ surface.mapRect.p2.x - 1, surface.mapRect.p2.y };
    while (rightTile.y < mapSize.y &&
        rightTile.x < mapSize.x &&
        getNextSideGroundTile(leftTile, mapSize.x, layer) &&
        getNextSideGroundTile(rightTile, mapSize.x, layer));
    surface.mapRect.p2 = rightTile;
    if(surface.mapRect.p2.y == mt.y) {
        std::cout << "Bad map! Did not parse a single row of ground tiles!" << std::endl;
        return false;
    }
    getRealPosFromMapPos(surface.mapRect.p1, surface.dimensions.p1, currentZ);
    getRealPosFromMapPos(surface.mapRect.p2, surface.dimensions.p2, currentZ + (surface.mapRect.p2.y - surface.mapRect.p1.y) );
    return true;
}

bool GameWindow::traceWallTiles(const mappoint& mt, tmx::Vector2u& mapSize, tmx::TileLayer &layer, int currentZ, SurfaceData &surface)
{
    TileType lastTileType = TileType::None;
    TileType curTileType = getTileType(mt, mapSize.x, layer);
    TileType expectedTileType = TileType::None;
    if(curTileType != TileType::Wall && curTileType != TileType::SideWallAngled1) {
        std::cout << "Bad map! Wall tiles organized in a way that tracer cannot trace the geometry!" << std::endl;
        return false;
    }
    surface.mapRect.p1 = surface.mapRect.p2 = mt;
    while(surface.mapRect.p2.y < mapSize.y && getTileType(surface.mapRect.p2, mapSize.x, layer) == TileType::Wall) {
        surface.mapRect.p2.y++;
    }
    surface.mapRect.p2.x++;
    while(surface.mapRect.p2.x < mapSize.x && 
        getTileType({ surface.mapRect.p2.x, surface.mapRect.p1.y }, mapSize.x, layer) == TileType::Wall &&
        getTileType({ surface.mapRect.p2.x, surface.mapRect.p2.y - 1 }, mapSize.x, layer) == TileType::Wall) {

        surface.mapRect.p2.x++;
    }
    getRealPosFromMapPos(surface.mapRect.p1, surface.dimensions.p1, currentZ);
    getRealPosFromMapPos(surface.mapRect.p2, surface.dimensions.p2, currentZ);
    return true;
}

/// @brief Trace a 3D ground surface from the 2D map by looking at the geometry of the tiles as defined in JSON
/// @param mapX X-coordinate on the 2D map
/// @param mapY Y-coordinate on the 2D map
/// @param layer Current layer being considered
/// @param currentZ derived Z-point in 3D space
/// @param surface Surface data to be written, containing the 3D collision data from the detected surface
bool GameWindow::traceSideWallTiles(const mappoint& mt, tmx::Vector2u& mapSize, tmx::TileLayer &layer, int currentZ, SurfaceData &surface)
{
    TileType lastTileType = TileType::None;
    TileType curTileType = getTileType(mt, mapSize.x, layer);
    TileType expectedTileType = TileType::None;
    if(curTileType != TileType::SideWallAngled1 && curTileType != TileType::SideWallAngled2) {
        std::cout << "Bad map! Side-wall tiles organized in a way that tracer cannot trace the geometry!" << std::endl;
        return false;
    }
    surface.mapRect.p1 = surface.mapRect.p2 = mt;
    while(surface.mapRect.p2.y < mapSize.y && isSideWallTile(getTileType(surface.mapRect.p2, mapSize.x, layer))) {
        surface.mapRect.p2.y++;
    }
    int y2_start = surface.mapRect.p2.y;
    TileType tileTmp = TileType::None;
    while(surface.mapRect.p2.x < layer.getSize().x && 
        surface.mapRect.p2.y < layer.getSize().y) {
        tileTmp = getTileType({ surface.mapRect.p2.x, surface.mapRect.p2.y - 1 }, mapSize.x, layer);
        if (tileTmp == TileType::SideWallAngled1 || tileTmp == TileType::SideWallAngled4) {
            surface.mapRect.p2.x++;
            surface.mapRect.p2.y++;
        } else if(tileTmp == TileType::SideWallAngled2 || tileTmp == TileType::SideWallAngled3) {
            surface.mapRect.p2.y++;
        } else {
            break;
        }
    }
    if (surface.mapRect.p2.y == mt.y) {
        std::cout << "Bad map! Did not parse a single column of side-wall tiles!" << std::endl;
        return false;
    } else {
        surface.mapRect.p2.x++;
    }
    getRealPosFromMapPos(surface.mapRect.p1, surface.dimensions.p1, currentZ);
    getRealPosFromMapPos(surface.mapRect.p2, surface.dimensions.p2, currentZ + (surface.mapRect.p2.y - y2_start) + 2);
    return true;
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
    uint64_t frameDeltaTime = curTime - lastFrameTime;
    SDL_RenderClear(renderer);
    for (const auto& l : renderLayers) {
        l->draw(renderer);
    }
    for (Actor* actor : actors) {
        actor->draw(this, frameDeltaTime / 1000.f);
    }
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    for (const auto& geometry : get_wall_geometries()) {
        SDL_RenderDrawLine(renderer, geometry.mapRect.p1.x * 16, geometry.mapRect.p1.y * 16, geometry.mapRect.p2.x * 16, geometry.mapRect.p2.y * 16);
    }
    for (const auto& geometry : get_ground_geometries()) {

        SDL_RenderDrawLine(renderer, geometry.mapRect.p1.x * 16, geometry.mapRect.p1.y * 16, geometry.mapRect.p2.x * 16, geometry.mapRect.p2.y * 16);
        
        int actualX1 = int((geometry.dimensions.p1.x + geometry.dimensions.p1.z / c_x_ratio));
        int actualY1 = int((geometry.dimensions.p1.z * 2 / c_x_ratio + geometry.dimensions.p1.y));
        int actualX2 = int((geometry.dimensions.p2.x + geometry.dimensions.p2.z / c_x_ratio));
        int actualY2 = int((geometry.dimensions.p2.z * 2 / c_x_ratio) + geometry.dimensions.p2.y);
        SDL_RenderDrawLine(renderer, actualX1 * 16, actualY1 * 16, actualX2 * 16, actualY2 * 16);
    }
    SDL_RenderPresent(renderer);
    lastFrameTime = curTime;
}

const std::vector<SurfaceData> GameWindow::get_wall_geometries() const 
{ 
    std::vector<SurfaceData> output;
    for (auto& surface : surfaces) {
        if (surface.layer == TileLayerId::ForegroundWall || surface.layer == TileLayerId::BackgroundWall) {
            output.push_back(surface);
        }
    }
    return output;
}

const std::vector<SurfaceData> GameWindow::get_ground_geometries() const
{
    std::vector<SurfaceData> output;
    for (auto& surface : surfaces) {
        if (surface.layer == TileLayerId::ForegroundWall || surface.layer == TileLayerId::BackgroundWall) {
            output.push_back(surface);
        }
    }
    return output;
}