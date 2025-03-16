#pragma once

#include "ActorState.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <memory>
#include "Geometry.h"
#include "GameWindow.h"

const float MAX_PLAYER_X_VELOCITY = 10.0f;
const float MIN_PLAYER_Y_VELOCITY = -40.f;
const float MAX_PLAYER_JUMP_VELOCITY = 10.f;
//const float MAX_PLAYER_Z_VELOCITY = 2.f;
const float PLAYER_RUN_ACCEL = 5.f; // 5 m/s^2
const float DEFAULT_JUMP_TIME = 0.5f;

class Actor
{
    class GameWindow& parentWindow;
    CollisionData collisions;
    cylinder collisionGeometry;
    cylinder collisionGeomCurrent;
    tripoint realpos;
    float jumpEndTime;
    float maxJumpTime;
    ActorState lastFrameState;
    MoveVector curMove;
    std::unique_ptr<class Texture> texture;
    std::unique_ptr<class SpriteProvider> spriteProvider;
    pixelpos windowPos;
protected:
    MoveVector intentMove;
	ActorState state;
	bool visible;

    void handleCollisions();
    virtual void handleMovement(float deltaTime, const MoveVector &intent, MoveVector &current);
    void handleGravity(float deltaTime);
    void handleJump(float deltaTime);

public:
	Actor( GameWindow& parentWindow, struct SpriteConfig& spriteConfig, Texture* texture, const mappoint &mt);
    virtual ~Actor();

	ActorState GetState() { return state; }
    const CollisionData& getCollisions() { return collisions; }
    void draw(float deltaTime, const pixelpos& camera);

    const tripoint &getRealPos() const { return realpos; } 

    const cylinder &getCollisionGeometry() const { return collisionGeomCurrent; }

    const pixelpos &getWindowPos() const { return windowPos; }
};

class PlayerActor : public Actor
{
    int intentKeys;
    int intentMoveKeys;

    void getMoveVectorFromMoveKey(int mKeysDown, MoveVector &mv);
    bool isMovementKey(SDL_Keycode keyCode);
    int getMovementTypeFromKey(SDL_Keycode keyCode);
    int getIntentFromKey(SDL_Keycode keyCode);

public:
    PlayerActor(GameWindow& parentWindow, SpriteConfig& spriteConfig, Texture* texture, const mappoint& mt);
    ~PlayerActor();

    void handle_input(const SDL_Event& event);
};