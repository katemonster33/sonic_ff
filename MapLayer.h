/*********************************************************************
(c) Matt Marchant 2024
http://trederia.blogspot.com

tmxlite - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#pragma once

/*
Encapsulates a texture and vertex array and uses them to draw a map layer
*/

#include "Texture.h"

#include <SDL.h>
#include <tmxlite/Map.hpp>
#include <vector>
#include "Actor.h"

struct tripoint
{
    float x;
    float y;
    float z;
};
struct TileData
{
    tripoint origin;
    tripoint endpoint;
};

const TileData ground {{0.0, 0.0, 0.0}, {1.0, 0.0, 1.0}};
const TileData wall {{0.0, 0.0, 0.0}, {1.0, 1.0, 0.0}};

const TileData box {{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}};

const TileData groundAngled1{{0.0, 0.0, 1.0}, {0.5, 0.0, 0.0}};

const TileData groundAngled2{{0.5, 0.0, 1.0}, {1.0, 0.0, 0.0}};

enum TileType
{
    None,
    Ground,
    Wall,
    GroundAngled1,
    GroundAngled2,
    GroundAngled3,
    GroundAngled4,
    
}

class MapLayer final
{


    std::vector<TileData> collisionGeometry;
public:
    explicit MapLayer();

    bool create(const tmx::Map&, std::uint32_t index, const std::vector<std::unique_ptr<Texture>>& textures);

    void draw(SDL_Renderer*) const;

private:
    struct Subset final
    {
        std::vector<SDL_Vertex> vertexData;
        SDL_Texture* texture = nullptr;
    };
    std::vector<Subset> m_subsets;
};