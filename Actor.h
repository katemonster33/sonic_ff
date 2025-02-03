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
    NoCollision,
    Left = 1,
    Right = 2,
    Up = 4,
    Down = 8,
    Front = 16,
    Back = 32
};

struct Rect2
{
    int x;
    int y;
    int w;
    int h;
};

struct Rect3 : public Rect2
{
    int z;
    int d;
};

struct Hitbox
{
    int x;
    int y;
    int z;

};

class Actor
{
    Rect2 spriteRect;
    Rect3 collisionRect;
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
	Actor( SpriteConfig *spriteConfig, Texture* texture, int startx, int starty, int startz);
    ~Actor();

    void handle_input(const SDL_Event& event);

	ActorState GetState() { return state; }

    CollisionType check_collision(GameWindow *parentWindow);
    void draw(GameWindow *parentWindow, uint64_t frameTimeDelta);
};