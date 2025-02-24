#pragma once

#include "SpriteSheet.h"
#include "ActorState.h"
#include "Texture.h"
#include "GameWindow.h"
#include <SDL2/SDL_events.h>
#include "Geometry.h"

const float MAX_PLAYER_X_VELOCITY = 5.0f;
const float MIN_PLAYER_Y_VELOCITY = -5.f;
const float MAX_PLAYER_JUMP_VELOCITY = 5.f;
const float MAX_PLAYER_Z_VELOCITY = 2.0f;
const float PLAYER_RUN_ACCEL = 5.0f; // 5 m/s^2


class Actor
{
    Rect2 spriteRect;
    Hitbox collisionGeometry;
    int mapX;
    int mapY;
    float x;
    float y;
    float z;
    float intentMoveAngle;
    float intentMovePercent;
    float curMoveAngle;
    float curMoveVelocity;
    float y_velocity;
    ActorState lastFrameState;
	ActorState state;
    int intent;
    int intentMoveKeys;
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
    void draw(GameWindow *parentWindow, float deltaQuotient);
};