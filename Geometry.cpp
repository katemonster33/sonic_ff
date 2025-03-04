#include "Geometry.h"

//#ifdef OS_WINDOWS
#define _USE_MATH_DEFINES
#include <math.h>
//#endif
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

inline bool line_intersects(float l1x1, float l1x2, float l2x1, float l2x2)
{
    return !(l1x1 > l2x2 || l1x2 < l1x1);
    //return (l2.x1 >= l1.x1 && l2.x1 <= l1.x2) ||
    //    (l2.x1 >= l1.x1 && l2.x2 <= l1.x2) ||
    //    (l1.x1 >= l2.x1 && l1.x1 <= l2.x2) ||
    //    (l1.x1 >= l2.x1 && l1.x2 <= l2.x2);
}

CollisionType get_collision(const cuboid& c1, const cuboid& c2)
{
    if (line_intersects(c1.p1.y, c1.p2.y, c2.p1.y, c2.p2.y) && 
        line_intersects(c1.p1.x, c1.p2.x, c2.p1.x, c2.p2.x) && 
        line_intersects(c1.p1.z, c1.p2.z, c2.p1.z, c2.p2.z)) {
        tripoint cpoint{ (c2.p1.x + c2.p2.x) / 2.f, (c2.p1.y + c2.p2.y) / 2.f, (c2.p1.z + c2.p2.z) / 2.f };
        if (cpoint.x < c1.p1.x) {
            return CollisionType::Left;
        } else if (cpoint.x > c1.p2.x) {
            return CollisionType::Right;
        } else if (cpoint.y < c1.p1.y) {
            return CollisionType::Up;
        } else if (cpoint.y > c1.p2.y) {
            return CollisionType::Down;
        } else if (cpoint.z < c1.p1.z) {
            return CollisionType::Front;
        } else if (cpoint.z > c1.p2.z) {
            return CollisionType::Back;
        }
        return (CollisionType)(CollisionType::Front | CollisionType::Back | CollisionType::Up | CollisionType::Down | CollisionType::Left | CollisionType::Right);
    }
    return CollisionType::NoCollision;
}

CollisionType get_collision(const cuboid& cube, const cylinder& cyl)
{
    if (line_intersects(cube.p1.y, cube.p2.y, cyl.y1, cyl.y2) && 
        circle_intersects_rect(cyl.x, cyl.z, cyl.r, (cube.p1.x + cube.p2.x) / 2, (cube.p1.z + cube.p2.z) / 2, cube.p2.x - cube.p1.x + 0.05, cube.p2.z - cube.p1.z + 0.05)) {
        float y_mid = (cyl.y2 + cyl.y1) / 2.f;
        if (y_mid < cube.p1.y) {
            return CollisionType::Down;
        } else if (y_mid > cube.p2.y) {
            return CollisionType::Up;
        } else {
            int colType = CollisionType::NoCollision;
            if (cyl.x < cube.p1.x) {
                colType |= CollisionType::Right;
            } else if (cyl.x > cube.p2.x) {
                colType |= CollisionType::Left;
            }
            if (cyl.z < cube.p1.z) {
                colType |= CollisionType::Front;
            } else if (cyl.z > cube.p2.z) {
                colType |= CollisionType::Back;
            }
            if (colType == CollisionType::NoCollision) {
                colType = CollisionType::Front | CollisionType::Back | CollisionType::Up | CollisionType::Down | CollisionType::Left | CollisionType::Right;
            }
            return (CollisionType)colType;
        }
    }
    return CollisionType::NoCollision;
}

bool circle_intersects_rect(float cx, float cy, float cr, float rx1, float ry1, float rw, float rh)
{
    float circleDistX = abs(cx - rx1);
    float circleDistY = abs(cy - ry1);

    if (circleDistX > (rw / 2 + cr) || circleDistY > (rh / 2 + cr)) { 
        return false; 
    }

    if (circleDistX <= (rw / 2) || circleDistY <= (rh / 2)) { 
        return true; 
    }

    float cornerDistSq = pow(circleDistX - rw / 2, 2.f) +
        pow(circleDistY - rh / 2, 2.f);

    return (cornerDistSq <= pow(cr, 2.f));
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

float triangle::degToRads(float degs)
{
    return degs * M_PI / 180;
}

float triangle::getCSide(float aSide, float bSide, float cAngleRads)
{
    return sqrt(bSide * bSide + aSide * aSide - (2 * bSide * aSide * cos(cAngleRads)));
}

float triangle::getAAngle(float aSide, float bSide, float cSide)
{
    return acos((cSide * cSide + bSide * bSide - aSide * aSide) / (2 * cSide * bSide));
}

void modifyVelocityFromTurn(float &curVelocity, float &curAngle, float intentAngle, float intentVelocity, float deltaVelocity)
{
    float deltaAngle = intentAngle - curAngle;
    if(deltaAngle > 180) {
        deltaAngle = intentAngle - (curAngle + 360);
    }
    float cSide = triangle::getCSide(curVelocity, intentVelocity, abs(triangle::degToRads(deltaAngle)));
    float aAngle = triangle::getAAngle(curVelocity, intentVelocity, cSide);
    float tmpCurVelocity = triangle::getCSide(curVelocity, cSide, aAngle);
    float newDeltaAngle = triangle::getAAngle(curVelocity, deltaVelocity, tmpCurVelocity);
    if(deltaAngle < 0) {
        curAngle -= newDeltaAngle;
    } else {
        curAngle += newDeltaAngle;
    }
    if(newDeltaAngle > abs(deltaAngle)) {
        curAngle = intentAngle;
    }
    curVelocity = tmpCurVelocity;
}