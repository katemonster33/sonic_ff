#include <unordered_map>
#include <string>
#include <vector>
#include "Geometry.h"

enum class TileType
{
    None,
    Ground,
    Box,
    Wall,
    GroundAngled1,
    GroundAngled2,
    GroundAngled3,
    GroundAngled4,
    SideWall,
    SideWallAngled1,
    SideWallAngled2,
    SideWallAngled3,
    SideWallAngled4
};

struct CustomTileObject
{
    std::string objectName;
    std::string typeName;
    std::vector<std::vector<int>> tileIds;
    cuboid bounds;
};

class TilesetConfig
{

    std::unordered_map<int, TileType> mapTileData;
    std::vector<CustomTileObject> customObjects;
public:
    TilesetConfig(struct cJSON *json);
    TileType getTileType(int tileId);
    const CustomTileObject* tryParseObject(int topLeftTileId);
    static TilesetConfig* Create(std::string path);
};