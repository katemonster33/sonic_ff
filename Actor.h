#pragma once

#include "ActorState.h"
#include "Texture.h"
#include "GameWindow.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>

#include "Geometry.h"

const float MAX_PLAYER_X_VELOCITY = 5.0f;
const float MIN_PLAYER_Y_VELOCITY = -5.f;
const float MAX_PLAYER_JUMP_VELOCITY = 5.f;
const float MAX_PLAYER_Z_VELOCITY = 2.0f;
const float PLAYER_RUN_ACCEL = 5.0f; // 5 m/s^2

struct MoveVector
{
    float angle;
    float velocity;
};


class Actor
{
    Rect2 spriteRect;
    Hitbox collisionGeometry;
    mappoint mappos;
    tripoint realpos;
    MoveVector intentMove;
    MoveVector curMove;
    float y_velocity;
    ActorState lastFrameState;
	ActorState state;
    int intent;
    int intentMoveKeys;
    Texture* texture;
    size_t spriteGroupIndex;
	struct SpriteConfig *spriteConfig;
    const struct SpriteGroup* activeGroup;
	bool visible;

    float getMoveAngleFromMoveKey(int mKeysDown);
    bool isMovementKey(SDL_Keycode keyCode);
    int getMovementTypeFromKey(SDL_Keycode keyCode);
    int getIntentFromKey(SDL_Keycode keyCode);
    void handleTurning(MoveVector &intent, MoveVector &current);

    void handleMovement(bool isRunning, float deltaTime, const MoveVector &intent, MoveVector &current);

public:
	Actor( SpriteConfig *spriteConfig, Texture* texture, const mappoint &mt);
    ~Actor();

    void handle_input(const SDL_Event& event);

	ActorState GetState() { return state; }

    CollisionType check_collision(GameWindow *parentWindow);
    void draw(GameWindow *parentWindow, float deltaQuotient);
};