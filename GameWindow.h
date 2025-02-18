#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <tmxlite/Types.hpp>
#include "Geometry.h"

const float gravity_accel = 9.8f; // 9.8 m/s^2

namespace tmx
{
    class Map;
    class TileLayer;
}

enum class TileType
{
    None,
    Ground,
    Box,
    Wall,
    GroundAngled1,
    GroundAngled2,
    GroundAngled3,
    GroundAngled4,
    SideWall,
    SideWallAngled1,
    SideWallAngled2,
    SideWallAngled3,
    SideWallAngled4
};

struct SurfaceData
{
    cuboid dimensions;
    rect mapRect;
};
class GameWindow
{
    int z0_x, z0_y;
    std::vector<SurfaceData> wallSurfaces, groundSurfaces;
    int getZLevelAtPoint(int mapX, int mapY);
    void traceGroundTiles(int mapX, int mapY, tmx::Vector2u& mapSize, tmx::TileLayer &layer, int currentZ, SurfaceData &surface);
    void traceSideWallTiles(int mapX, int mapY, tmx::Vector2u& mapSize, tmx::TileLayer &layer, int currentZ, SurfaceData &surface);
    void traceWallTiles(int mapX, int mapY, tmx::Vector2u& mapSize, tmx::TileLayer &layer, int currentZ, SurfaceData &surface);
    tmx::TileLayer *getLayerByName(const char *name);
    SurfaceData *createSurfaceFromMap(int mapX, int mapY, tmx::Vector2u &mapSize, const tmx::TileLayer &layer, int currentZ);
    TileType getTileType(int mapX, int mapY, int mapSizeX, const tmx::TileLayer &layer);
    std::unordered_map<int, TileType> mapTileData;
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
    bool readJsonTileData();
    bool any_surface_intersects(const std::vector<SurfaceData> &surfaces, int mapX, int mapY);
public:
    static GameWindow *Create();
    ~GameWindow();

    struct SDL_Renderer *getRenderer() { return renderer;}
    size_t GetSizeX() { return size_x; }
    size_t GetSizeY() { return size_y; }
    tripoint getTripointAtMapPoint(int mapX, int mapY);

    void handle_input(const union SDL_Event& event);
    const std::vector<SurfaceData> get_wall_geometries() const { return wallSurfaces; }
    const std::vector<SurfaceData> get_ground_geometries() const { return groundSurfaces; }
    int getHeight(int x, int y);
    void drawFrame();
};