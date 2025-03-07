#pragma once 
#include <cmath>
#define _USE_MATH_DEFINES
#include <math.h>

// helper variable for determining the actual position of the actor given a z-offset and trying to determine the x- and y-offset
const float c_x_ratio = 2.f;//sqrt(5);
const float M_PI_F = float(M_PI);

struct Rect2
{
    int x;
    int y;
    int w;
    int h;
};

struct MoveVector
{
    // float angle;
    // float velocity;
    float x;
    float y;
    float z;
};

struct mappoint
{
    unsigned int x;
    unsigned int y;
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

namespace triangle
{
    float degToRads(float degs);
    float getCSide(float aSide, float bSide, float cAngleRads);
    float getAAngle(float aSide, float bSide, float cSide);
}

CollisionType get_collision(const cuboid& cube, const cylinder& cyl);
CollisionType get_collision(const cylinder& cyl1, const cylinder& cyl2);

bool line_intersects(float l1x1, float l1x2, float l2x1, float l2x2);
bool circle_intersects_rect(float cx, float cy, float cr, float rx1, float ry1, float rw, float rh);

void getPixelPosFromRealPos(const tripoint &realpos, int &pixelX, int &pixelY);
void getMapPosFromRealPos(const tripoint &realpos, mappoint &mappos);
void getRealPosFromMapPos(const mappoint &mappos, tripoint &realpoint, float z);

void modifyVelocityFromTurn(const MoveVector& intentVec, MoveVector& curVec, float deltaVelocity);