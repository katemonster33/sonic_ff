#pragma once

#include "SpriteSheet.h"
#include "ActorState.h"
#include "Texture.h"
#include "GameWindow.h"
#include <SDL2/SDL_events.h>

class Actor
{
    int x;
    int y;
    float x_velocity;
    float y_velocity;
	ActorState state;
    Texture* texture;
    size_t spriteGroupIndex;
	SpriteConfig *spriteConfig;
    const SpriteGroup* activeGroup;
	bool visible;
public:
	Actor( SpriteConfig *spriteConfig, Texture* texture, size_t startx, size_t starty);
    ~Actor();

    void handle_input(const SDL_Event& event);

	ActorState GetState() { return state; }

    void draw(GameWindow *parentWindow);
};