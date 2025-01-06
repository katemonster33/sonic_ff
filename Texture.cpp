#include "Texture.h"
#include <iostream>
#include "GameWindow.h"
#include <SDL2/SDL_image.h>

Texture::Texture(GameWindow *parentWindow, SDL_Surface *surface, SDL_Texture *texture)
{
    this->parentWindow = parentWindow;
    this->surface = surface;
    this->texture = texture;
}

Texture *Texture::Create(GameWindow* parentWindow, std::string filename)
{
    SDL_Surface *surface = IMG_Load(filename.c_str());
    if(!surface)
    {
        SDL_Log("Failed to create surface from file %s : %s", filename.c_str(), SDL_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(parentWindow->getRenderer(), surface);
    if(!texture)
    {
        SDL_Log("Failed to create texture from surface: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return nullptr;
    }

    return new Texture(parentWindow, surface, texture);
}

Texture::~Texture()
{
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void Texture::draw(int x, int y, int w, int h, SDL_RendererFlip flip)
{
    SDL_Rect srcRect = {0, 0, w, h};
    SDL_Rect destRect = {x, y, w, h};

    SDL_RenderCopyEx(parentWindow->getRenderer(), texture, &srcRect, &destRect, 0.0, NULL, flip);
}

void Texture::draw(int srcX, int srcY, int destX, int destY, int w, int h, SDL_RendererFlip flip)
{
    SDL_Rect srcRect = {srcX, srcY, w, h};
    SDL_Rect destRect = {destX, destY, w, h};

    SDL_RenderCopyEx(parentWindow->getRenderer(), texture, &srcRect, &destRect, 0.0, NULL, flip);
}