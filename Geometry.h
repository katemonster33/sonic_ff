#pragma once 
#include <cmath>

// helper variable for determining the actual position of the actor given a z-offset and trying to determine the x- and y-offset
const double c_x_ratio = 2;//sqrt(5);

struct Rect2
{
    int x;
    int y;
    int w;
    int h;
};

struct Hitbox
{
    float x;
    float y;
    float w;
    float h;
};

struct mappoint
{
    int x;
    int y;
};

struct tripoint
{
    float x;
    float y;
    float z;
};

/// @brief 2D rectangular surface 
struct maprect
{
    mappoint p1;
    mappoint p2;

    bool intersects(const maprect& other) const;
    bool intersects(const mappoint &p) const;
};

/// @brief 3D cuboid surface, for handling absolute dimensions of a collidable surface
struct cuboid
{
    tripoint p1;
    tripoint p2;
};

struct cylinder
{
    float x;
    float y1;
    float y2;
    float z;
    float r;
};

enum CollisionType
{
    NoCollision,
    Left = 1,
    Right = 2,
    Up = 4,
    Down = 8,
    Front = 16,
    Back = 32
};

CollisionType get_collision(const cuboid& cube, const cylinder& cyl);
CollisionType get_collision(const cylinder& cyl1, const cylinder& cyl2);

void getPixelPosFromRealPos(const tripoint &realpos, int &pixelX, int &pixelY);
void getMapPosFromRealPos(const tripoint &realpos, mappoint &mappos);
void getRealPosFromMapPos(const mappoint &mappos, tripoint &realpoint, int z);