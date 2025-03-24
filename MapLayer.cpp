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

#include "MapLayer.h"

#include <tmxlite/TileLayer.hpp>

#include <iostream>
#include <array>
#include <cassert>
#include <cJSON/cJSON.h>

MapLayer::MapLayer()
{
}

bool MapLayer::create(const tmx::Map& map, std::uint32_t layerIndex, const std::vector<std::unique_ptr<Texture>>& textures)
{
    const auto& layers = map.getLayers();
    assert(layers[layerIndex]->getType() == tmx::Layer::Type::Tile);

    const auto& layer = layers[layerIndex]->getLayerAs<tmx::TileLayer>();
    const auto mapSize = map.getTileCount();
    const auto mapTileSize = map.getTileSize();
    const auto& tileSets = map.getTilesets();

    const auto tintColour = layer.getTintColour();
    const SDL_Colour vertColour =
    {
        tintColour.r,
        tintColour.g,
        tintColour.b,
        tintColour.a
    };

    for (auto i = 0u; i < tileSets.size(); ++i)
    {
        //check tile ID to see if it falls within the current tile set
        const auto& ts = tileSets[i];
        const auto& tileIDs = layer.getTiles();

        const auto texSize = textures[i]->getSize();
        const auto tileCountX = texSize.x / mapTileSize.x;
        const auto tileCountY = texSize.y / mapTileSize.y;

        const float uNorm = static_cast<float>(mapTileSize.x) / texSize.x;
        const float vNorm = static_cast<float>(mapTileSize.y) / texSize.y;

        std::vector<SDL_Vertex> verts;
        for (auto y = 0u; y < mapSize.y; ++y)
        {
            for (auto x = 0u; x < mapSize.x; ++x)
            {
                const auto idx = y * mapSize.x + x;
                if (idx < tileIDs.size() && tileIDs[idx].ID >= ts.getFirstGID()
                    && tileIDs[idx].ID < (ts.getFirstGID() + ts.getTileCount()))
                {
                    //tex coords
                    auto idIndex = (tileIDs[idx].ID - ts.getFirstGID());
                    float u = static_cast<float>(idIndex % tileCountX);
                    float v = static_cast<float>(idIndex / tileCountY);
                    u *= mapTileSize.x; //TODO we should be using the tile set size, as this may be different from the map's grid size
                    v *= mapTileSize.y;

                    //normalise the UV
                    u /= textures[i]->getSize().x;
                    v /= textures[i]->getSize().y;

                    //vert pos
                    const float tilePosX = static_cast<float>(x) * mapTileSize.x;
                    const float tilePosY = (static_cast<float>(y) * mapTileSize.y);

                    // push back to vert array
                    if(tileIDs[idx].flipFlags == tmx::TileLayer::FlipFlag::Vertical) {
                        verts.insert(verts.end(), {
                            { { tilePosX, tilePosY + mapTileSize.y }, vertColour, {u, v} }, 
                            { { tilePosX + mapTileSize.x, tilePosY + mapTileSize.y }, vertColour, {u + uNorm, v} },
                            { { tilePosX, tilePosY }, vertColour, {u, v + vNorm} },
                            { { tilePosX, tilePosY}, vertColour, {u, v + vNorm} },
                            { { tilePosX + mapTileSize.x, tilePosY + mapTileSize.y }, vertColour, {u + uNorm, v} },
                            { { tilePosX + mapTileSize.x, tilePosY }, vertColour, {u + uNorm, v + vNorm} }
                        });
                    } else if(tileIDs[idx].flipFlags == tmx::TileLayer::FlipFlag::Horizontal) {
                        verts.insert(verts.end(), {
                            { { tilePosX + mapTileSize.x, tilePosY }, vertColour, {u, v} }, 
                            { { tilePosX, tilePosY }, vertColour, {u + uNorm, v} },
                            { { tilePosX + mapTileSize.x, tilePosY + mapTileSize.y}, vertColour, {u, v + vNorm} },
                            { { tilePosX + mapTileSize.x, tilePosY + mapTileSize.y}, vertColour, {u, v + vNorm} },
                            { { tilePosX, tilePosY }, vertColour, {u + uNorm, v} },
                            { { tilePosX, tilePosY + mapTileSize.y }, vertColour, {u + uNorm, v + vNorm} }
                        });
                    // } else if(tileIDs[idx].flipFlags == tmx::TileLayer::FlipFlag::Diagonal) {
                        // verts.insert(verts.end(), {
                        //     { { tilePosX + mapTileSize.x, tilePosY + mapTileSize.y  }, vertColour, {u, v} }, 
                        //     { { tilePosX, tilePosY + mapTileSize.y  }, vertColour, {u + uNorm, v} },
                        //     { { tilePosX + mapTileSize.x, tilePosY}, vertColour, {u, v + vNorm} },
                        //     { { tilePosX + mapTileSize.x, tilePosY}, vertColour, {u, v + vNorm} },
                        //     { { tilePosX, tilePosY + mapTileSize.y  }, vertColour, {u + uNorm, v} },
                        //     { { tilePosX, tilePosY }, vertColour, {u + uNorm, v + vNorm} }
                        // });
                    } else if(tileIDs[idx].flipFlags == (tmx::TileLayer::FlipFlag::Vertical | tmx::TileLayer::FlipFlag::Horizontal)) {
                        verts.insert(verts.end(), {
                            { { tilePosX + mapTileSize.x, tilePosY + mapTileSize.y  }, vertColour, {u, v} }, 
                            { { tilePosX, tilePosY + mapTileSize.y  }, vertColour, {u + uNorm, v} },
                            { { tilePosX + mapTileSize.x, tilePosY}, vertColour, {u, v + vNorm} },
                            { { tilePosX + mapTileSize.x, tilePosY}, vertColour, {u, v + vNorm} },
                            { { tilePosX, tilePosY + mapTileSize.y  }, vertColour, {u + uNorm, v} },
                            { { tilePosX, tilePosY }, vertColour, {u + uNorm, v + vNorm} }
                        });
                    } else {
                        verts.insert(verts.end(), {
                            { { tilePosX, tilePosY }, vertColour, {u, v} }, 
                            { { tilePosX + mapTileSize.x, tilePosY }, vertColour, {u + uNorm, v} },
                            { { tilePosX, tilePosY + mapTileSize.y}, vertColour, {u, v + vNorm} },
                            { { tilePosX, tilePosY + mapTileSize.y}, vertColour, {u, v + vNorm} },
                            { { tilePosX + mapTileSize.x, tilePosY }, vertColour, {u + uNorm, v} },
                            { { tilePosX + mapTileSize.x, tilePosY + mapTileSize.y }, vertColour, {u + uNorm, v + vNorm} }
                        });
                    }
                }
            }
        }

        if (!verts.empty())
        {
            m_subsets.emplace_back();
            m_subsets.back().texture = *textures[i];
            m_subsets.back().vertexData.swap(verts);
        }
    }

    return true;
}

void MapLayer::draw(SDL_Renderer* renderer, int cameraX, int cameraY) const
{
    assert(renderer);
    for(const auto& subset : m_subsets) {
        std::vector<SDL_Vertex> vertsCpy;
        for(const auto& vertex: subset.vertexData) {
            vertsCpy.push_back({ {vertex.position.x - cameraX, vertex.position.y - cameraY}, vertex.color, vertex.tex_coord });
        }
        SDL_RenderGeometry(renderer, subset.texture, vertsCpy.data(), static_cast<std::int32_t>(vertsCpy.size()), nullptr, 0);
    }
}