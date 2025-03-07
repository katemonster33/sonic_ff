#pragma once

#include "ActorState.h"
#include "Texture.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <memory>
#include "Geometry.h"
#include "GameWindow.h"

const float MAX_PLAYER_X_VELOCITY = 5.0f;
const float MIN_PLAYER_Y_VELOCITY = -5.f;
const float MAX_PLAYER_JUMP_VELOCITY = 0.5f;
const float MAX_PLAYER_Z_VELOCITY = 2.0f;
const float PLAYER_RUN_ACCEL = 5.0f; // 5 m/s^2
const float DEFAULT_JUMP_TIME = 0.3f;

class Actor
{
    CollisionData collisions;
    cylinder collisionGeomCurrent;
protected:
    float jumpEndTime;
    float maxJumpTime;
    class GameWindow& parentWindow;
    Rect2 spriteRect;
    cylinder collisionGeometry;
    tripoint realpos;
    MoveVector intentMove;
    MoveVector curMove;
    float y_velocity;
    ActorState lastFrameState;
	ActorState state;
    std::unique_ptr<Texture> texture;
    size_t spriteGroupIndex;
	std::unique_ptr<struct SpriteConfig> spriteConfig;
    struct SpriteGroup& activeGroup;
	bool visible;

    void handleCollisions();
    virtual void handleMovement(float deltaTime, const MoveVector &intent, MoveVector &current);
    void handleGravity(float deltaTime);
    void handleJump(float deltaTime);

public:
	Actor( GameWindow& parentWindow, SpriteConfig* spriteConfig, Texture* texture, const mappoint &mt);
    virtual ~Actor();

	ActorState GetState() { return state; }
    const CollisionData& getCollisions() { return collisions; }
    void draw(float deltaTime);
};

class PlayerActor : public Actor
{
    int intentKeys;
    int intentMoveKeys;

    MoveVector getMoveVectorFromMoveKey(int mKeysDown);
    bool isMovementKey(SDL_Keycode keyCode);
    int getMovementTypeFromKey(SDL_Keycode keyCode);
    int getIntentFromKey(SDL_Keycode keyCode);

protected:
    void handleMovement(float deltaTime, const MoveVector& intent, MoveVector& current);
public:
    PlayerActor(GameWindow& parentWindow, SpriteConfig* spriteConfig, Texture* texture, const mappoint& mt);
    ~PlayerActor();

    void handle_input(const SDL_Event& event);
};