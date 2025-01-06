#include <string>
#include <SDL2/SDL_render.h>

class Texture
{
    class GameWindow* parentWindow;
    SDL_Surface *surface;
    SDL_Texture *texture;
    Texture(GameWindow* parentWindow, SDL_Surface *surface, SDL_Texture *texture);
public:
    static Texture *Create(GameWindow* parentWindow, std::string filename);
    ~Texture();

    void draw(int x, int y, int w, int h, SDL_RendererFlip flip = SDL_FLIP_NONE);
    void draw(int srcX, int srcY, int destX, int destY, int w, int h, SDL_RendererFlip flip = SDL_FLIP_NONE);
};