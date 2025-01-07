#pragma once

#include "SpriteSheet.h"
#include "ActorState.h"
#include "Texture.h"
#include "GameWindow.h"
#include <SDL2/SDL_events.h>

class Actor
{
    size_t x;
    size_t y;
	ActorState state;
    Texture* texture;
    size_t spriteGroupIndex;
	SpriteConfig *spriteConfig;
    const SpriteGroup* activeGroup;
	bool visible;
public:
	Actor( SpriteConfig *spriteConfig, Texture* texture, size_t x, size_t y);
    ~Actor();

    void handle_input(SDL_Event* event);

	ActorState GetState() { return state; }

    void draw(GameWindow *parentWindow);
};