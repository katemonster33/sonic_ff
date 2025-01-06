#include "SpriteSheet.h"
#include <iostream>
#include <string>
#include <SDL2/SDL_image.h>

SpriteConfig sonicSpriteCfg{
  "Sonic",
  {
    {
      ActorState::Default,
      {
        { 0, 0, 30, 42 }
      }
    },
    {
      ActorState::Idle,
      {
        { 30, 0, 31, 42 },
        { 61, 0, 32, 42 },
        { 94, 0, 33, 42 },
        { 127, 0, 33, 42 },
        { 160, 0, 33, 42 },
        { 193, 0, 33, 42 },
        { 226, 0, 34, 42 },
        { 260, 0, 37, 42 },
        { 297, 0, 34, 42 },
        { 330, 0, 33, 42 },
        { 363, 0, 28, 42 },
        { 391, 0, 28, 42 }
      }
    },
    { 
      ActorState::LookingUp, 
      {
        { 419, 0, 33, 42 }, { 452, 0, 28, 42 }
      }
    }
  }
};

SDL_Surface *loadAndOptimizeSurface(SDL_PixelFormat *format, std::string path)
{
  //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface != nullptr )
    {
        std::cout << "Unable to load image " << path.c_str() << "! SDL_image Error: " << IMG_GetError() << std::endl;
	return nullptr;
    }
    else
    {
        //Convert surface to screen format
        SDL_Surface *outputSurface = SDL_ConvertSurface( loadedSurface, format, 0 );

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
        if( outputSurface == nullptr )
        {
            std::cout << "Unable to optimize image " << path.c_str() << "! SDL Error: " << SDL_GetError() << std::endl;
        }
        return outputSurface;
    }
}

SpriteSheet::SpriteSheet(SDL_PixelFormat *format, std::string path)
{
  spriteSurface = loadAndOptimizeSurface(format, path);
  activeSpriteIndex = 0;
  
}

void SpriteSheet::AddSprite(SDL_Rect* sprite)
{
  sprites.push_back(sprite);
}

void SpriteSheet::AddSpriteAction(std::string name, std::vector< size_t > spriteIndices)
{
  spriteActionsByName[name] = spriteIndices;
}

void SpriteSheet::AddSpriteRow(SDL_Rect* firstSprite, size_t numSprites)
{
  SDL_Rect *tempRect = new SDL_Rect { firstSprite->x, firstSprite->y, firstSprite->w, firstSprite->h };
  for(size_t index = 0; index < numSprites; index++)
  {
    AddSprite(tempRect);
  }
}

SDL_Rect* SpriteSheet::GetActiveSprite()
{
  if(activeSpriteIndex < 0 || activeSpriteIndex >= sprites.size())
  {
    return nullptr;
  }
  return sprites[activeSpriteIndex];
}

bool SpriteSheet::HasUpdate(uint32_t millis)
{
  return false;
}