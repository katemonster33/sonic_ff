#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <tmxlite/Types.hpp>

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

/// @brief 3D point meant for normalized dimensions for tiles (e.g. values can only be 0-1 as in 0-100% tile side length)
struct tripoint
{
    float x;
    float y;
    float z;
};

/// @brief 2D rectangular surface 
struct rect
{
    int x1;
    int y1;
    int x2;
    int y2;

    bool intersects(const rect &other) const;
    bool intersects(int x, int y) const;
};

/// @brief 3D cuboid surface, for handling absolute dimensions of a collidable surface
struct cuboid
{
    int x1;
    int y1;
    int z1;
    int x2;
    int y2;
    int z2;
};

struct TileData
{
    tripoint origin;
    tripoint endpoint;
    TileType tileType;
    int id;
};

struct SurfaceData
{
    cuboid dimensions;
    rect mapRect;
};

const TileData ground {{0.0, 0.0, 0.0}, {1.0, 0.0, 1.0}, TileType::Ground, 0};
const TileData wall {{0.0, 0.0, 0.0}, {1.0, 1.0, 0.0}, TileType::Wall, 0};

const TileData box {{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, TileType::Box, 0};

const TileData groundAngled1{{0.0, 0.0, 1.0}, {0.5, 0.0, 0.0}, TileType::GroundAngled1, 0};

const TileData groundAngled2{{0.5, 0.0, 1.0}, {1.0, 0.0, 0.0}, TileType::GroundAngled2, 0};

class GameWindow
{
    std::vector<SurfaceData> collisionGeometry;
    void traceGroundTiles(int mapX, int mapY, tmx::Vector2u& mapSize, tmx::TileLayer &layer, int currentZ, SurfaceData &surface);
    void traceSideWallTiles(int mapX, int mapY, tmx::Vector2u& mapSize, tmx::TileLayer &layer, int currentZ, SurfaceData &surface);
    void traceWallTiles(int mapX, int mapY, tmx::Vector2u& mapSize, tmx::TileLayer &layer, int currentZ, SurfaceData &surface);
    tmx::TileLayer *getLayerByName(const char *name);
    SurfaceData *createSurfaceFromMap(int mapX, int mapY, tmx::Vector2u &mapSize, const tmx::TileLayer &layer, int currentZ);
    TileType getTileType(int mapX, int mapY, int mapSizeX, const tmx::TileLayer &layer);
    std::unordered_map<int, TileData> mapTileData;
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

    void handle_input(const union SDL_Event& event);
    int getHeight(int x, int y);
    void drawFrame();
};