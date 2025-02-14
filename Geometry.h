#pragma once 

struct Rect2
{
    int x;
    int y;
    int w;
    int h;
};

struct Rect3 : public Rect2
{
    int z;
    int d;
};

struct Hitbox
{
    int x;
    int y;
    int w;
    int h;
};

/// @brief 2D rectangular surface 
struct rect
{
    int x1;
    int y1;
    int x2;
    int y2;

    bool intersects(const rect& other) const;
    bool intersects(int x, int y) const;
};

/// @brief 3D cuboid surface, for handling absolute dimensions of a collidable surface
struct cuboid
{
    float x1;
    float y1;
    float z1;
    float x2;
    float y2;
    float z2;
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
//CollisionType check_collision(const cylinder& cyl1, const cylinder& cyl2);