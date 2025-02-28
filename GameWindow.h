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

enum class TileLayerId
{
    None,
    BackgroundWall,
    ForegroundWall,
    Ground,
    Obstacle,
    Any
};

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
    TileLayerId layer;
    cuboid dimensions;
    maprect mapRect;
};
class GameWindow
{
    int z0_x, z0_y;
    std::vector<SurfaceData> surfaces;
    int getZLevelAtPoint(const mappoint &mt, TileLayerId layer = TileLayerId::Any);
    bool getNextSideGroundTile(mappoint& mt, tmx::TileLayer& layer);
    bool traceGroundTiles(const mappoint& mt, tmx::TileLayer &layer, int currentZ, SurfaceData &surface);
    bool traceSideWallTiles(const mappoint& mt, tmx::TileLayer &layer, int currentZ, SurfaceData &surface);
    bool traceWallTiles(const mappoint& mt, tmx::TileLayer &layer, int currentZ, SurfaceData &surface);
    tmx::TileLayer *getLayerByName(const char *name);
    TileType getTileType(const mappoint& mt, const tmx::TileLayer &layer);
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
    const tmx::Vector2u& mapSize;

    GameWindow(SDL_Window *window, SDL_Renderer *renderer, tmx::Map& map, size_t sizex, size_t sizey);
    bool readJsonTileData();
    bool any_surface_intersects(const std::vector<SurfaceData> &surfaces, const mappoint &mt);
public:
    static GameWindow *Create();
    ~GameWindow();

    struct SDL_Renderer *getRenderer() { return renderer;}
    size_t GetSizeX() { return size_x; }
    size_t GetSizeY() { return size_y; }
    tripoint getTripointAtMapPoint(const mappoint& mt);

    void handle_input(const union SDL_Event& event);
    const std::vector<SurfaceData> get_wall_geometries() const;
    const std::vector<SurfaceData> get_ground_geometries() const;
    void drawFrame();
};