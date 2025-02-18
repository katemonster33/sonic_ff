#pragma once

#include "SpriteSheet.h"
#include "ActorState.h"
#include "Texture.h"
#include "GameWindow.h"
#include <SDL2/SDL_events.h>
#include "Geometry.h"

const float MAX_PLAYER_X_VELOCITY = 5.0f;
const float MAX_PLAYER_Z_VELOCITY = 2.0f;



class Actor
{
    Rect2 spriteRect;
    Hitbox collisionGeometry;
    int mapX;
    int mapY;
    int x;
    int y;
    int height;
    int jump_height;
    float jump_velocity;
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
	Actor( SpriteConfig *spriteConfig, Texture* texture, int mapX, int mapY);
    ~Actor();

    void handle_input(const SDL_Event& event);

	ActorState GetState() { return state; }

    CollisionType check_collision(GameWindow *parentWindow);
    void draw(GameWindow *parentWindow, uint64_t frameTimeDelta);
};