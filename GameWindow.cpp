#include "GameWindow.h"
#include "SpriteSheet.h"
#include "Actor.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "MapLayer.h"
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
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

bool GameWindow::any_surface_intersects(TileLayerId surfaceType, const mappoint& mt)
{
    for(const auto& surface : surfaces) {
        if((surfaceType == TileLayerId::Any || surface.layer == surfaceType) && surface.mapRect.intersects(mt)) {
            return true;
        }
    }
    return false;
}

tripoint GameWindow::getTripointAtMapPoint(const mappoint& mt)
{
    float zlevel = getZLevelAtPoint(mt);
    tripoint output{ -1.f, -1.f, -1.f };
    if (zlevel != -1) {
        getRealPosFromMapPos(mt, output, zlevel);
    }
    return output;
}

float GameWindow::getZLevelAtPoint(const mappoint& mt, TileLayerId layer)
{
    if (layer == TileLayerId::Ground || layer == TileLayerId::Any) {
        for (const SurfaceData& groundSurface : surfaces) {
            if (groundSurface.layer == TileLayerId::Ground && groundSurface.mapRect.intersects(mappoint{ mt.x, mt.y - 1 })) {
                return groundSurface.dimensions.p1.z + (mt.y - groundSurface.mapRect.p1.y);
            }
        }
    }
    for(const auto &surface : surfaces) {
        if (surface.layer != TileLayerId::Ground && (surface.layer == layer || layer == TileLayerId::Any)) {
            if (surface.mapRect.intersects(mappoint{ mt.x, mt.y - 1 })) {
                if (surface.dimensions.p2.z > (surface.dimensions.p1.z + 1)) {
                    return (float(mt.x) - surface.mapRect.p1.x) / 2;
                } else {
                    return  surface.dimensions.p2.z;
                }
            } else if (surface.mapRect.intersects(mappoint{ mt.x + 1, mt.y })) {
                //currentZ = surface.dimensions.p2.z - surface.dimensions.p1.z + 
            } else if (surface.layer == TileLayerId::ForegroundWall && 
                surface.mapRect.intersects(mappoint{ mt.x - 1, mt.y }) && 
                surface.mapRect.p2.x == mt.x) {
                return surface.dimensions.p2.z;
            }
        }
    }
    return -1;
}

GameWindow::GameWindow(SDL_Window *window, SDL_Renderer *renderer, tmx::Map& map, pixelpos size) : 
    camera{ 0, 0 },
    size(size),
    map(map),
    curTime(0),
    lastFrameTime(0),
    z0pos{ 0, 0 },
    mapSize(map.getTileCount()),
    window(window),
    renderer(renderer),
    tilesetConfig(TilesetConfig::Create(std::string("assets/") + map.getTilesets()[0].getName() + ".json"))
{
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
    auto bgLayer = getLayerByName("Background");
    const auto& mapSize = map.getTileCount();
    std::vector<SurfaceData> miscSurfaces;
    if(bgLayer != nullptr) {
        float currentZ = 0;
        mappoint mt{ 0, 0 };
        for (; mt.x < mapSize.x; ++mt.x) {
            for (mt.y = 0; mt.y < mapSize.y; ++mt.y) {
                TileType bgTileType = getTileType(mt, *bgLayer);
                if(bgTileType == TileType::Wall || bgTileType == TileType::SideWallAngled1) {
                    SurfaceData wallSurface;
                    wallSurface.layer = TileLayerId::BackgroundWall;
                    bool traceSuccess = false;
                    if(bgTileType == TileType::Wall) {
                        traceSuccess = traceWallTiles(mt, *bgLayer, currentZ, wallSurface);
                    } else {
                        traceSuccess = traceSideWallTiles(mt, *bgLayer, currentZ, wallSurface);
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
                TileType bgTileType = getTileType(mt, *fgWallLayer);
                if(bgTileType == TileType::Wall || bgTileType == TileType::SideWallAngled1) {
                    SurfaceData wallSurface;
                    wallSurface.layer = TileLayerId::ForegroundWall;
                    bool parseSuccess = false;
                    float zOffset = 0;
                    if(bgTileType == TileType::Wall) {
                        zOffset = getZLevelAtPoint({ mt.x, mt.y }, TileLayerId::ForegroundWall);
                        parseSuccess = traceWallTiles(mt, *fgWallLayer, zOffset, wallSurface);
                    } else {
                        zOffset = getZLevelAtPoint({ mt.x - 1, mt.y }, TileLayerId::BackgroundWall);
                        parseSuccess = traceSideWallTiles(mt, *fgWallLayer, zOffset, wallSurface);
                    }
                    if (parseSuccess) {
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
                TileType fgTileType = getTileType(mt, *fgLayer);
                if(isGroundTile(fgTileType) && !any_surface_intersects(TileLayerId::Ground, mt)) {
                    float currentZ = getZLevelAtPoint(mt);
                    SurfaceData fgSurface;
                    fgSurface.layer = TileLayerId::Ground;
                    traceGroundTiles(mt, *fgLayer, currentZ, fgSurface);
                    surfaces.push_back(fgSurface);
                }
            }
        }
    }
    auto collLayer = getLayerByName("collidables");
    if(collLayer != nullptr) {
        mappoint mt{ 0, 0 };
        for(; mt.x < mapSize.x; mt.x++) {
            for(mt.y = 0; mt.y < mapSize.y; mt.y++) {
                TileType fgTileType = getTileType(mt, *collLayer);
                if(fgTileType == TileType::Box && !any_surface_intersects(TileLayerId::Obstacle, mt)) {
                    float currentZ = getZLevelAtPoint(mt);
                    SurfaceData fgSurface;
                    fgSurface.layer = TileLayerId::Obstacle;
                    traceBoxTiles(mt, *collLayer, currentZ, fgSurface);
                    surfaces.push_back(fgSurface);
                }
            }
        }
    }
    z0pos = { surfaces[0].mapRect.p1.x, surfaces[0].mapRect.p1.y };

    playerActor = new PlayerActor(*this, &sonicSpriteCfg, Texture::Create(renderer, "assets/images/sonic3.png"), { 13, 11 });
}

GameWindow::~GameWindow()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

TileType GameWindow::getTileType(const mappoint &mt, const tmx::TileLayer &layer)
{
    int tileId = layer.getTiles()[mapSize.x * mt.y + mt.x].ID;
    if(tileId == 0) {
        return TileType::None;
    }
    int fgid = map.getTilesets()[0].getFirstGID();
    int lgid = map.getTilesets()[0].getLastGID();
    if(tileId >= fgid && tileId <= lgid) {
        tileId -= fgid;
    }
    return tilesetConfig->getTileType(tileId);
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

bool GameWindow::getNextSideGroundTile(mappoint& mt, tmx::TileLayer& layer)
{
    TileType tileTmp = getTileType(mt, layer);
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

bool GameWindow::traceBoxTiles(const mappoint& mt, tmx::TileLayer &layer, float currentZ, SurfaceData &surface)
{
    TileType lastTileType = TileType::None;
    TileType curTileType = getTileType(mt, layer);
    TileType expectedTileType = TileType::None;
    if(curTileType != TileType::Box) {
        std::cout << "Bad map! Ground tiles organized in a way that tracer cannot trace the geometry! [" << mt.x << "," << mt.y << "]" << std::endl;
        return false;
    }
    surface.mapRect.p1 = surface.mapRect.p2 = mt;
    
    while(surface.mapRect.p2.x < mapSize.x && getTileType(surface.mapRect.p2, layer) == TileType::Box) {
        surface.mapRect.p2.x++;
    }
    surface.mapRect.p2.y++;
    while (surface.mapRect.p2.y < mapSize.y &&
        surface.mapRect.p2.x < mapSize.x &&
    getTileType({surface.mapRect.p2.x - 1, surface.mapRect.p2.y - 1}, layer) == TileType::Box) {
        surface.mapRect.p2.y++;
    }
    if(surface.mapRect.p2.y == mt.y) {
        std::cout << "Bad map! Did not parse a single row of box tiles! [" << surface.mapRect.p2.x << "," << surface.mapRect.p2.y << "]" << std::endl;
        return false;
    }
    getRealPosFromMapPos(surface.mapRect.p1, surface.dimensions.p1, currentZ);
    getRealPosFromMapPos(surface.mapRect.p2, surface.dimensions.p2, currentZ + 1);
    return true;
}

bool GameWindow::traceGroundTiles(const mappoint& mt, tmx::TileLayer &layer, float currentZ, SurfaceData &surface)
{
    surface.layer = TileLayerId::Ground;
    TileType lastTileType = TileType::None;
    TileType curTileType = getTileType(mt, layer);
    TileType expectedTileType = TileType::None;
    if(curTileType != TileType::GroundAngled1 && curTileType != TileType::GroundAngled2) {
        std::cout << "Bad map! Ground tiles organized in a way that tracer cannot trace the geometry! [" << mt.x << "," << mt.y << "]" << std::endl;
        return false;
    }
    surface.mapRect.p1 = surface.mapRect.p2 = mt;
    
    while(surface.mapRect.p2.x < mapSize.x && isGroundTile(getTileType(surface.mapRect.p2, layer))) {
        surface.mapRect.p2.x++;
    }
    mappoint leftTile = mt;
    mappoint rightTile{ surface.mapRect.p2.x - 1, surface.mapRect.p2.y };
    while (rightTile.y < mapSize.y &&
        rightTile.x < mapSize.x &&
        getNextSideGroundTile(leftTile, layer) &&
        getNextSideGroundTile(rightTile, layer));
    surface.mapRect.p2 = rightTile;
    if(surface.mapRect.p2.y == mt.y) {
        std::cout << "Bad map! Did not parse a single row of ground tiles! [" << surface.mapRect.p2.x << "," << surface.mapRect.p2.y << "]" << std::endl;
        return false;
    }
    getRealPosFromMapPos(surface.mapRect.p1, surface.dimensions.p1, currentZ);
    getRealPosFromMapPos(surface.mapRect.p2, surface.dimensions.p2, currentZ + (surface.mapRect.p2.y - surface.mapRect.p1.y) );
    return true;
}

bool GameWindow::traceWallTiles(const mappoint& mt, tmx::TileLayer &layer, float currentZ, SurfaceData &surface)
{
    TileType lastTileType = TileType::None;
    TileType curTileType = getTileType(mt, layer);
    TileType expectedTileType = TileType::None;
    if(curTileType != TileType::Wall && curTileType != TileType::SideWallAngled1) {
        std::cout << "Bad map! Wall tiles organized in a way that tracer cannot trace the geometry!" << std::endl;
        return false;
    }
    surface.mapRect.p1 = surface.mapRect.p2 = mt;
    while(surface.mapRect.p2.y < mapSize.y && getTileType(surface.mapRect.p2, layer) == TileType::Wall) {
        surface.mapRect.p2.y++;
    }
    surface.mapRect.p2.x++;
    while(surface.mapRect.p2.x < mapSize.x && 
        getTileType({ surface.mapRect.p2.x, surface.mapRect.p1.y }, layer) == TileType::Wall &&
        getTileType({ surface.mapRect.p2.x, surface.mapRect.p2.y - 1 }, layer) == TileType::Wall) {

        surface.mapRect.p2.x++;
    }
    getRealPosFromMapPos(surface.mapRect.p1, surface.dimensions.p1, currentZ);
    getRealPosFromMapPos(surface.mapRect.p2, surface.dimensions.p2, currentZ);
    return true;
}

/// @brief Trace a 3D ground surface from the 2D map by looking at the geometry of the tiles as defined in JSON
/// @param mt coordinate on the 2D map
/// @param layer Current layer being considered
/// @param currentZ derived Z-point in 3D space
/// @param surface Surface data to be written, containing the 3D collision data from the detected surface
bool GameWindow::traceSideWallTiles(const mappoint& mt, tmx::TileLayer &layer, float currentZ, SurfaceData &surface)
{
    TileType lastTileType = TileType::None;
    TileType curTileType = getTileType(mt, layer);
    TileType expectedTileType = TileType::None;
    if(curTileType != TileType::SideWallAngled1 && curTileType != TileType::SideWallAngled2) {
        std::cout << "Bad map! Side-wall tiles organized in a way that tracer cannot trace the geometry!" << std::endl;
        return false;
    }
    surface.mapRect.p1 = surface.mapRect.p2 = mt;
    while(surface.mapRect.p2.y < mapSize.y && isSideWallTile(getTileType(surface.mapRect.p2, layer))) {
        surface.mapRect.p2.y++;
    }
    surface.mapRect.p2.y--;
    int y2_start = surface.mapRect.p2.y;
    TileType tileTmp = TileType::None, lastTile = TileType::None;
    while(surface.mapRect.p2.x < layer.getSize().x && 
        surface.mapRect.p2.y < layer.getSize().y) {
        tileTmp = getTileType({ surface.mapRect.p2.x, surface.mapRect.p2.y }, layer);
        if (tileTmp == TileType::SideWallAngled1 || tileTmp == TileType::SideWallAngled4) {
            surface.mapRect.p2.x++;
            surface.mapRect.p2.y++;
        } else if(tileTmp == TileType::SideWallAngled2 || tileTmp == TileType::SideWallAngled3) {
            surface.mapRect.p2.y++;
        } else {
            break;
        }
        lastTile = tileTmp;
    }
    if (surface.mapRect.p2.y == mt.y) {
        std::cout << "Bad map! Did not parse a single column of side-wall tiles!" << std::endl;
        return false;
    } 
    getRealPosFromMapPos(surface.mapRect.p1, surface.dimensions.p1, currentZ);
    getRealPosFromMapPos(surface.mapRect.p2, surface.dimensions.p2, currentZ + (surface.mapRect.p2.y - y2_start) + 1);
    return true;
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
    return new GameWindow(window, renderer, map, { 852, 480 });
}

void GameWindow::handle_input(const SDL_Event& event)
{
    if (playerActor != nullptr) {
        playerActor->handle_input(event);
    }
}

void GameWindow::drawFrame()
{
    curTime = SDL_GetTicks64();
    float frameDeltaTime = float(curTime - lastFrameTime) / 1000.f;
    SDL_RenderClear(renderer);
    camera.x = playerActor->getWindowPos().x - (size.x / 2);
    camera.y = playerActor->getWindowPos().y - (size.y / 2);
    for (const auto& l : renderLayers) {
        l->draw(renderer, camera.x, camera.y);
    }
    if (playerActor != nullptr) {
        playerActor->draw(frameDeltaTime, camera);
    }
    for (Actor* actor : actors) {
        actor->draw(frameDeltaTime, camera);
    }
    SDL_RenderPresent(renderer);
    lastFrameTime = curTime;
}

cylinder collisionCylCpy;
const CollisionData GameWindow::check_collision(const cylinder& collisionCyl)
{
    CollisionData collisions;
    collisions.directions = CollisionType::NoCollision;
    for (const auto& geometry : get_geometries()) {
        int cTypeTmp = get_collision(geometry.dimensions, collisionCyl);
        if (cTypeTmp != CollisionType::NoCollision) {
            collisions.directions |= cTypeTmp;
            collisions.collisions.push_back(CollisionData::CollisionItem{ cTypeTmp, geometry });
        }
    }
    return collisions;
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
        if (surface.layer == TileLayerId::Ground) {
            output.push_back(surface);
        }
    }
    return output;
}

const std::vector<SurfaceData> GameWindow::get_obstacle_geometries() const
{
    std::vector<SurfaceData> output;
    for (auto& surface : surfaces) {
        if (surface.layer == TileLayerId::Obstacle) {
            output.push_back(surface);
        }
    }
    return output;
}