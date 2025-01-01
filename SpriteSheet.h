#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <unordered_map>

class SpriteSheet
{
  SDL_Surface *spriteSurface;
  size_t activeSpriteIndex;
  std::vector<SDL_Rect*> sprites;
  std::unordered_map<std::string, std::vector<size_t>> spriteActionsByName;
public:
  SpriteSheet(SDL_PixelFormat *format, std::string path);
  
  bool HasUpdate(uint32_t millis);
  SDL_Rect *GetActiveSprite();
  
protected:
  void AddSprite(SDL_Rect *sprite);
  void AddSpriteRow(SDL_Rect *firstSprite, size_t numSprites);
  void AddSpriteAction(std::string name, std::vector<size_t> spriteIndices);
};