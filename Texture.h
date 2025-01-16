#pragma once

#include <string>
#include <SDL.h>

class Texture final
{
    struct SDL_Renderer* renderer;
    struct SDL_Texture *texture;
    SDL_Point m_size;
    Texture(SDL_Renderer* renderer, SDL_Texture *texture, SDL_Point& size);
public:
    static Texture *Create(SDL_Renderer* renderer, std::string filename);

    Texture(const Texture&) = delete;
    Texture(Texture&&) = delete;

    Texture& operator = (const Texture&) = delete;
    Texture& operator = (Texture&&) = delete;

    SDL_Point getSize() const { return m_size; }
    ~Texture();
    operator SDL_Texture* () { return texture; }

    void draw(int x, int y, int w, int h, SDL_RendererFlip flip = SDL_FLIP_NONE);
    void draw(int srcX, int srcY, int destX, int destY, int w, int h, SDL_RendererFlip flip = SDL_FLIP_NONE);
};