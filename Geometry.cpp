#include "Geometry.h"
#include <cmath>


bool maprect::intersects(const maprect &other) const
{
    return ((p1.x >= other.p1.x && p1.x <= other.p2.x) || (p2.x >= other.p1.x && p2.x <= other.p2.x)) && 
        ((p1.y >= other.p1.y && p1.y <= other.p2.y) || (p2.y >= other.p1.y && p2.y <= other.p2.y));
}

bool maprect::intersects(const mappoint &p) const
{
    return p.x <= p2.x && p.x >= p1.x && 
    p.y <= p2.y && p.y >= p1.y;
}

float triangulate(float a, float b)
{
    return sqrt((a * a) + (b * b));
}

CollisionType get_collision(const cuboid& cube, const cylinder& cyl)
{
    int cType = CollisionType::NoCollision;
    if ((cyl.y1 >= cube.p1.y && cyl.y1 <= cube.p2.y) ||
            (cyl.y2 >= cube.p1.y && cyl.y2 <= cube.p2.y)) {
        if (cyl.x >= cube.p1.x && cyl.x <= cube.p2.x &&
            cyl.z >= cube.p1.z && cyl.z <= cube.p2.z) {
            if (cyl.x >= cube.p1.x) {
                cType |= CollisionType::Left;
            }
            if (cyl.x <= cube.p2.x) {
                cType |= CollisionType::Right;
            }
            if (cyl.z >= cube.p1.z) {
                cType |= CollisionType::Front;
            }
            if (cyl.z <= cube.p2.z) {
                cType |= CollisionType::Back;
            }

        }
        else if (cyl.x < cube.p1.x && (cyl.x + cyl.r) >= cube.p1.x) {
            int xdist = cube.p1.x - cyl.x;
            if ((cyl.z + cyl.r) >= cube.p1.z && triangulate(xdist, cube.p1.z - cyl.z) < cyl.r) {
                cType = CollisionType::Front | CollisionType::Left;
            }
            else if ((cyl.z - cyl.r) <= cube.p2.z && triangulate(xdist, cyl.z - cube.p2.z) < cyl.r) {
                cType = CollisionType::Back | CollisionType::Left;
            }
        }
        else if (cyl.x > cube.p2.x && (cyl.x - cyl.r) <= cube.p2.x) {
            int xdist = cyl.x - cube.p2.x;
            if ((cyl.z + cyl.r) >= cube.p1.z && triangulate(xdist, cube.p1.z - cyl.z) < cyl.r) {
                cType = CollisionType::Front | CollisionType::Right;
            }
            else if ((cyl.z - cyl.r) <= cube.p2.z && triangulate(xdist, cyl.z - cube.p2.z)) {
                cType = CollisionType::Back | CollisionType::Right;
            }
        }
        if (cType != CollisionType::NoCollision) {
            if (cyl.y2 == cube.p1.y) {
                cType = CollisionType::Down;
            }
            else if (cyl.y1 == cube.p2.y) {
                cType = CollisionType::Up;
            }
            else {
                if (cyl.y2 > cube.p1.y) {
                    cType |= CollisionType::Down;
                }
                if (cyl.y1 < cube.p2.y) {
                    cType |= CollisionType::Up;
                }
            }
        }
    }
    return (CollisionType)cType;
}

CollisionType get_collision(const cylinder& cyl1, const cylinder& cyl2)
{
   int cType = CollisionType::NoCollision;
   if ((cyl1.y1 >= cyl2.y1 && cyl1.y1 <= cyl2.y2) ||
           (cyl1.y2 >= cyl2.y1 && cyl1.y2 <= cyl2.y2)) {
        int xdiff = cyl1.x - cyl2.x;
        int zdiff = cyl1.z - cyl2.z;
        if(triangulate(xdiff, zdiff) <= (cyl1.r + cyl2.r)) {
            if(cyl1.x <= cyl2.x) {
                cType |= CollisionType::Left;
            } else {
                cType |= CollisionType::Right;
            }
            if(cyl1.z <= cyl2.z) {
                cType |= CollisionType::Front;
            } else {
                cType |= CollisionType::Back;
            }
        }
       if (cType != CollisionType::NoCollision) {
           if (cyl1.y2 == cyl2.y1) {
               cType = CollisionType::Up;
           }
           else if (cyl1.y1 == cyl2.y2) {
               cType = CollisionType::Down;
           }
           else {
               if (cyl1.y2 > cyl2.y1) {
                   cType |= CollisionType::Up;
               }
               if (cyl1.y1 < cyl2.y2) {
                   cType |= CollisionType::Down;
               }
           }
       }
   }
   return (CollisionType)cType;
}

void getPixelPosFromRealPos(const tripoint &realpos, int &pixelX, int &pixelY)
{
    pixelX = int((realpos.x + realpos.z / c_x_ratio) * 16);
    pixelY = int((realpos.z + realpos.y) * 16);
}

void getMapPosFromRealPos(const tripoint &realpos, mappoint &mappos)
{
    mappos.x = int(realpos.x + realpos.z / c_x_ratio);
    mappos.y = int(realpos.z + realpos.y);
}

void getRealPosFromMapPos(const mappoint &mappos, tripoint &realpoint, int z)
{
    realpoint.x = mappos.x - float(z) / c_x_ratio;
    realpoint.y = mappos.y - float(z);
    realpoint.z = z;
}