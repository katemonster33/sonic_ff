#pragma once

#include <vector>
#include <string>
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