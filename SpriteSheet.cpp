#include "SpriteSheet.h"
#include <iostream>
#include <SDL2/SDL_image.h>

SDL_Surface *loadAndOptimizeSurface(SDL_PixelFormat *format, std::__cxx11::string path)
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

SpriteSheet::SpriteSheet(SDL_PixelFormat *format, std::__cxx11::string path)
{
  spriteSurface = loadAndOptimizeSurface(format, path);
  activeSpriteIndex = 0;
  
}

void SpriteSheet::AddSprite(SDL_Rect* sprite)
{
  sprites.push_back(sprite);
}

void SpriteSheet::AddSpriteAction(std::__cxx11::string name, std::vector< size_t > spriteIndices)
{
  spriteActionsByName.insert(name, spriteIndices);
}

void SpriteSheet::AddSpriteRow(SDL_Rect* firstSprite, size_t numSprites)
{
  SDL_Rect *tempRect = new SDL_Rect { firstSprite->x, firstSprite->y, firstSprite->w, firstSprite->h };
  for(size_t index = 0; index < numSprites; index++)
  {
    AddSprite(tempRect);
  }
}

SDL_Surface* SpriteSheet::GetActiveSprite()
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





