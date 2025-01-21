#pragma once

#include "SpriteSheet.h"
#include "ActorState.h"
#include "Texture.h"
#include "GameWindow.h"
#include <SDL2/SDL_events.h>

const float MAX_PLAYER_X_VELOCITY = 5.0f;
const float MAX_PLAYER_Z_VELOCITY = 2.0f;

enum CollisionType
{
    None,
    Left = 1,
    Right = 2,
    Up = 4,
    Down = 8
};

class Actor
{
    int x;
    int y;
    int z;
    float x_velocity;
    float y_velocity;
    float z_velocity;
    ActorState lastFrameState;
	ActorState state;
    int intent;
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

    CollisionType check_collision(GameWindow *parentWindow);
    void draw(GameWindow *parentWindow, time_t frameTimeDelta);
};