#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <tmxlite/Types.hpp>
#include "Geometry.h"
#include "TilesetConfig.h"

const float gravity_accel = 4.f;//9.8f; // 9.8 m/s^2

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

struct SurfaceData
{
    TileLayerId layer;
    cuboid dimensions;
    maprect mapRect;
};

struct CollisionData
{
    struct CollisionItem
    {
        int direction = 0;
        SurfaceData surface;
    };
    int directions = 0;
    std::vector<CollisionItem> collisions;
};

class GameWindow
{
    pixelpos camera;
    mappoint z0pos;
    std::unique_ptr<TilesetConfig> tilesetConfig;
    std::vector<SurfaceData> surfaces;
    float getZLevelAtPoint(const mappoint &mt, TileLayerId layer = TileLayerId::Any);
    bool getNextSideGroundTile(mappoint& mt, tmx::TileLayer& layer);
    bool traceBoxTiles(const mappoint& mt, tmx::TileLayer &layer, float currentZ, SurfaceData &surface);
    bool traceGroundTiles(const mappoint& mt, tmx::TileLayer &layer, float currentZ, SurfaceData &surface);
    bool traceSideWallTiles(const mappoint& mt, tmx::TileLayer &layer, float currentZ, SurfaceData &surface);
    bool traceWallTiles(const mappoint& mt, tmx::TileLayer &layer, float currentZ, SurfaceData &surface);
    tmx::TileLayer *getLayerByName(const char *name);
    TileType getTileType(const mappoint& mt, const tmx::TileLayer &layer);
    struct SDL_Window *window;
    struct SDL_Renderer *renderer;
    std::vector<std::unique_ptr<class MapLayer>> renderLayers;
    std::vector<std::unique_ptr<class Texture>> textures;
    tmx::Map& map;
    pixelpos size;
    class PlayerActor* playerActor;
    std::vector<class Actor*> actors;
    uint64_t curTime;
    uint64_t lastFrameTime;
    const tmx::Vector2u& mapSize;

    GameWindow(SDL_Window *window, SDL_Renderer *renderer, tmx::Map& map, pixelpos size);
    bool any_surface_intersects(TileLayerId surfaceType, const mappoint &mt);
public:
    static GameWindow *Create();
    ~GameWindow();

    struct SDL_Renderer *getRenderer() { return renderer;}
    const pixelpos& GetSize() { return size; }
    tripoint getTripointAtMapPoint(const mappoint& mt);

    void handle_input(const union SDL_Event& event);

    const CollisionData check_collision(const cylinder& collisionCyl);

    const std::vector<SurfaceData> get_wall_geometries() const;
    const std::vector<SurfaceData> get_ground_geometries() const;
    const std::vector<SurfaceData> get_obstacle_geometries() const;
    const std::vector<SurfaceData> get_geometries() const { return surfaces; }
    void drawFrame();
};