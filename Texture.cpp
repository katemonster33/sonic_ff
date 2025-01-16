#include "Texture.h"
#include <iostream>
#include "GameWindow.h"
#include <SDL2/SDL_image.h>
#include <SDL_rect.h>

Texture::Texture(SDL_Renderer* renderer, SDL_Texture *texture, SDL_Point& size) : 
    renderer(renderer),
    texture(texture),
    m_size(size)
{
}

Texture *Texture::Create(SDL_Renderer* renderer, std::string filename)
{
    SDL_Texture* texture = IMG_LoadTexture(renderer, filename.c_str());
    if(!texture)
    {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
        return nullptr;
    }
    SDL_Point sizeTmp;
    if (SDL_QueryTexture(texture, NULL, NULL, &sizeTmp.x, &sizeTmp.y) != 0)
    {
        SDL_Log("Failed to query texture size: %s", SDL_GetError());
        SDL_DestroyTexture(texture);
        return nullptr;
    }
    

    return new Texture(renderer, texture, sizeTmp);
}

Texture::~Texture()
{
    SDL_DestroyTexture(texture);
}

void Texture::draw(int x, int y, int w, int h, SDL_RendererFlip flip)
{
    SDL_Rect srcRect = {0, 0, w, h};
    SDL_Rect destRect = {x, y, w, h};

    SDL_RenderCopyEx(renderer, texture, &srcRect, &destRect, 0.0, NULL, flip);
}

void Texture::draw(int srcX, int srcY, int destX, int destY, int w, int h, SDL_RendererFlip flip)
{
    SDL_Rect srcRect = {srcX, srcY, w, h};
    SDL_Rect destRect = {destX, destY, w, h};

    SDL_RenderCopyEx(renderer, texture, &srcRect, &destRect, 0.0, NULL, flip);
}