#pragma once

#include <vector>
#include <string>
#include <memory>
#include "Geometry.h"
#include "ActorState.h"
#include <SDL2/SDL_render.h>

struct SpriteGroup
{
  ActorState groupState;
  std::vector<SDL_Rect> sprites;
};

struct SpriteConfig
{
  std::string name;
  std::vector<SpriteGroup> spriteGroups;
};

struct SpriteSheetConfig
{
  std::vector<SpriteConfig> sprites;
};

class SpriteProvider
{
    size_t spriteGroupIndex;
    std::unique_ptr<struct SpriteConfig> spriteConfig;
    Rect2 spriteRect;
    const struct SpriteGroup* activeGroup;
public:
    SpriteProvider(SpriteConfig* spriteConfig);

    void update(ActorState currentState);
    void draw();
    SDL_Rect getRect() {
        return activeGroup->sprites[spriteGroupIndex];
    };
};