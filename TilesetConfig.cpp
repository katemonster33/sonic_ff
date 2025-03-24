#include "TilesetConfig.h"
#include <cJSON/cJSON.h>
#include <fstream>
#include <cstring>

TileType TilesetConfig::getTileType(int tileId)
{
    TileType tileType = TileType::None;
    std::unordered_map<int, TileType>::iterator it = mapTileData.find(tileId);
    if (it != mapTileData.end()) {
        tileType = it->second;
    }
    return tileType;
}


tripoint readTripoint(cJSON* tripointObj)
{
    tripoint output{ 0.0f, 0.0f, 0.0f };
    for (cJSON* tripointAttr = tripointObj->child; tripointAttr != nullptr; tripointAttr = tripointAttr->next) {
        if (strcmp(tripointAttr->string, "x") == 0) {
            output.x = float(tripointAttr->valuedouble);
        }
        else if (strcmp(tripointAttr->string, "y") == 0) {
            output.y = float(tripointAttr->valuedouble);
        }
        else if (strcmp(tripointAttr->string, "z") == 0) {
            output.z = float(tripointAttr->valuedouble);
        }
    }
    return output;
}

cuboid readBounds(cJSON* boundsObj)
{
    cuboid output{ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
    for (cJSON* boundAttr = boundsObj->child; boundAttr != nullptr; boundAttr = boundAttr->next) {
        if (strcmp(boundAttr->string, "p1") == 0) {
            output.p1 = readTripoint(boundAttr);
        }
        else if (strcmp(boundAttr->string, "p2") == 0) {
            output.p2 = readTripoint(boundAttr);
        }
    }
    return output;
}

TilesetConfig::TilesetConfig(cJSON *json)
{
    for (cJSON* childJson = json->child; childJson != nullptr; childJson = childJson->next) {
        if (strcmp(childJson->string, "predefinedBasicTiles") == 0) {

            for (cJSON* predefinedTileObj = childJson->child; predefinedTileObj != nullptr; predefinedTileObj = predefinedTileObj->next) {
                TileType typeToSet = TileType::None;
                if (strcmp(predefinedTileObj->string, "wall") == 0) {
                    typeToSet = TileType::Wall;
                }
                else if (strcmp(predefinedTileObj->string, "sideWall") == 0) {
                    typeToSet = TileType::SideWall;
                }
                else if (strcmp(predefinedTileObj->string, "ground") == 0) {
                    typeToSet = TileType::Ground;
                }
                else if (strcmp(predefinedTileObj->string, "box") == 0) {
                    typeToSet = TileType::Box;
                }
                for (cJSON* tileIds = predefinedTileObj->child; tileIds != nullptr; tileIds = tileIds->next) {
                    mapTileData[tileIds->valueint] = typeToSet;
                }
            }
        }
        else if (strcmp(childJson->string, "predefinedAngledTiles") == 0) {
            for (cJSON* predefinedTileObj = childJson->child; predefinedTileObj != nullptr; predefinedTileObj = predefinedTileObj->next) {
                std::vector<TileType> tileTypes;
                if (strcmp(predefinedTileObj->string, "angledSideWall") == 0) {
                    tileTypes = { TileType::SideWallAngled1, TileType::SideWallAngled2, TileType::SideWallAngled3, TileType::SideWallAngled4 };
                }
                else if (strcmp(predefinedTileObj->string, "angledGround") == 0) {
                    tileTypes = { TileType::GroundAngled1, TileType::GroundAngled2, TileType::GroundAngled3, TileType::GroundAngled4 };
                }
                for (cJSON* tileIdArrays = predefinedTileObj->child; tileIdArrays != nullptr; tileIdArrays = tileIdArrays->next) {
                    auto tileType = tileTypes.begin();
                    for (cJSON* tileId = tileIdArrays->child; tileId != nullptr && tileType != tileTypes.end(); tileId = tileId->next, tileType++) {
                        if (tileId->valueint != 0) {
                            mapTileData[tileId->valueint] = *tileType;
                        }
                    }
                }
            }
        } else if (strcmp(childJson->string, "customTiles") == 0) {

        } else if (strcmp(childJson->string, "customObjects")) {
            for (cJSON* customObj = childJson->child; customObj != nullptr; customObj = customObj->next) {
                char* tileName = customObj->string;
                CustomTileObject cto;
                cto.typeName = tileName;
                for (cJSON* customObjAttr = customObj->child; customObjAttr != nullptr; customObjAttr = customObjAttr->next) {
                    if (strcmp(customObjAttr->string, "tileIds") == 0) {
                        for (cJSON* tileIdLst = customObjAttr->child; tileIdLst != nullptr; tileIdLst = tileIdLst->next) {
                            std::vector<int>& tileIdsTmp = cto.tileIds.emplace_back();
                            for (cJSON* tileId = tileIdLst->child; tileId != nullptr; tileId = tileId->next) {
                                tileIdsTmp.push_back(tileId->valueint);
                            }
                        }
                    } else if (strcmp(customObjAttr->string, "bounds") == 0) {
                        cto.bounds = readBounds(customObjAttr);
                    } else if (strcmp(customObjAttr->string, "type") == 0) {
                        cto.typeName = customObjAttr->valuestring;
                    }
                }
                customObjects.push_back(cto);
            }
        }
    }
}

const CustomTileObject* TilesetConfig::tryParseObject(int topLeftTileId)
{
    for (const auto& objTmp : customObjects) {
        if (objTmp.tileIds[0][0] == topLeftTileId) {
            return &objTmp;
        }
    }
    return nullptr;
}

size_t getFileLength(std::ifstream& file)
{
    file.seekg(0, std::ios::end);
    size_t length = file.tellg();
    file.seekg(0, std::ios::beg);
    return length;
}

TilesetConfig* TilesetConfig::Create(std::string path)
{
    std::ifstream jsonFile(path, std::ios::in);
    if (!jsonFile.is_open()) {
        return nullptr;
    }

    size_t len = getFileLength(jsonFile);
    char* fileData = new char[len];
    jsonFile.read(fileData, len);
    jsonFile.close();

    cJSON* json = cJSON_Parse(fileData);
    delete[] fileData;
    if (json == nullptr) {
        return nullptr;
    }
    else if (json->child == nullptr) {
        cJSON_Delete(json);
        return nullptr;
    }
    TilesetConfig* tscfg = new TilesetConfig(json);
    cJSON_Delete(json);
    return tscfg;
}