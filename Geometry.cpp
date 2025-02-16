#include "Geometry.h"
#include <cmath>


bool rect::intersects(const rect &other) const
{
    return ((x1 >= other.x1 && x1 <= other.x2) || (x2 >= other.x1 && x2 <= other.x2)) && 
        ((y1 >= other.y1 && y1 <= other.y2) || (y2 >= other.y1 && y2 <= other.y2));
}

bool rect::intersects(int x, int y) const
{
    return x <= x2 && x >= x1 && 
    y <= y2 && y >= y1;
}

float triangulate(float a, float b)
{
    return sqrt((a * a) + (b * b));
}

CollisionType get_collision(const cuboid& cube, const cylinder& cyl)
{
    int cType = CollisionType::NoCollision;
    if ((cyl.y1 >= cube.y1 && cyl.y1 <= cube.y2) ||
            (cyl.y2 >= cube.y1 && cyl.y2 <= cube.y2)) {
        if (cyl.x >= cube.x1 && cyl.x <= cube.x2 &&
            cyl.z >= cube.z1 && cyl.z <= cube.z2) {
            if (cyl.x >= cube.x1) {
                cType |= CollisionType::Left;
            }
            if (cyl.x <= cube.x2) {
                cType |= CollisionType::Right;
            }
            if (cyl.z >= cube.z1) {
                cType |= CollisionType::Front;
            }
            if (cyl.z <= cube.z2) {
                cType |= CollisionType::Back;
            }

        }
        else if ((cyl.x + cyl.r) >= cube.x1) {
            int xdist = cube.x1 - cyl.x;
            if ((cyl.z + cyl.r) >= cube.z1 && triangulate(xdist, cube.z1 - cyl.z) < cyl.r) {
                cType = CollisionType::Front | CollisionType::Left;
            }
            else if ((cyl.z - cyl.r) <= cube.z2 && triangulate(xdist, cyl.z - cube.z2) < cyl.r) {
                cType = CollisionType::Back | CollisionType::Left;
            }
        }
        else if ((cyl.x - cyl.r) <= cube.x2) {
            int xdist = cyl.x - cube.x2;
            if ((cyl.z + cyl.r) >= cube.z1 && triangulate(xdist, cube.z1 - cyl.z) < cyl.r) {
                cType = CollisionType::Front | CollisionType::Right;
            }
            else if ((cyl.z - cyl.r) <= cube.z2 && triangulate(xdist, cyl.z - cube.z2)) {
                cType = CollisionType::Back | CollisionType::Right;
            }
        }
        if (cType != CollisionType::NoCollision) {
            if (cyl.y2 == cube.y1) {
                cType = CollisionType::Up;
            }
            else if (cyl.y1 == cube.y2) {
                cType = CollisionType::Down;
            }
            else {
                if (cyl.y2 > cube.y1) {
                    cType |= CollisionType::Up;
                }
                if (cyl.y1 < cube.y2) {
                    cType |= CollisionType::Down;
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