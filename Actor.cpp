#include "Actor.h"
#include "SpriteSheet.h"
#include "Geometry.h"
#include "GameWindow.h"
#include "Texture.h"
#include <cassert>

Actor::Actor(GameWindow& parentWindow, SpriteConfig& spriteConfig, Texture* texture, const mappoint &mt) :
    spriteProvider(std::make_unique<SpriteProvider>(spriteConfig)),
    texture(texture),
    parentWindow(parentWindow),
    realpos(parentWindow.getTripointAtMapPoint(mt)),
    intentMove( MoveVector{0.f, 0.f, 0.f} ),
    curMove( MoveVector{0.f, 0.f, 0.f} ),
    collisionGeometry({-1.f, -1.f, -1.f, -1.f}),
    state(ActorState::Default),
    lastFrameState(ActorState::Default),
    visible(true),
    maxJumpTime(DEFAULT_JUMP_TIME),
    jumpEndTime(-1.f),
    collisions({0}),
    collisionGeomCurrent({ -1.f, -1.f, -1.f, -1.f, -1.f })
{
}

Actor::~Actor()
{
}

PlayerActor::PlayerActor(GameWindow& parentWindow, SpriteConfig& spriteConfig, Texture* texture, const mappoint& mt) : 
    Actor(parentWindow, spriteConfig, texture, mt),
    intentKeys(NoIntent),
    intentMoveKeys(MKeyNone)
{
}

PlayerActor::~PlayerActor()
{
}

bool PlayerActor::isMovementKey(SDL_Keycode keyCode)
{
    switch (keyCode) {
        case SDLK_UP:
        case SDLK_w:
        case SDLK_DOWN:
        case SDLK_s:
        case SDLK_LEFT:
        case SDLK_a:
        case SDLK_RIGHT:
        case SDLK_d:
            return true;
        default:
            return false;
        }
}

int PlayerActor::getMovementTypeFromKey(SDL_Keycode keyCode)
{
    switch (keyCode) {
        case SDLK_UP:
        case SDLK_w:
            return MKeyUp;
        case SDLK_DOWN:
        case SDLK_s:
            return MKeyDown;
        case SDLK_LEFT:
        case SDLK_a:
            return MKeyLeft;
        case SDLK_RIGHT:
        case SDLK_d:
            return MKeyRight;
        default:
            return MKeyNone;
        }
}

int PlayerActor::getIntentFromKey(SDL_Keycode keyCode)
{
    switch(keyCode)
    {
        case SDLK_UP:
        case SDLK_w:
        case SDLK_DOWN:  
        case SDLK_s:
        case SDLK_LEFT:
        case SDLK_a:
        case SDLK_RIGHT:
        case SDLK_d:
            return Run;
        case SDLK_LCTRL:
            return Attack;
        case SDLK_SPACE:
            return Jump;
        default:
            return NoIntent;
    }
}

void PlayerActor::getMoveVectorFromMoveKey(int mKeysDown, MoveVector &mv)
{
    switch(mKeysDown) {
        case MKeyUp:
        default:
            mv.x = 0.f;
            mv.z = -MAX_PLAYER_X_VELOCITY;
            break;
        case MKeyUp | MKeyRight:
            mv.x = MAX_PLAYER_X_VELOCITY / 2;
            mv.z = -MAX_PLAYER_X_VELOCITY / 2;
            break;
        case MKeyRight:
            mv.x = MAX_PLAYER_X_VELOCITY;
            mv.z = 0.f;
            break;
        case MKeyRight | MKeyDown:
            mv.x = MAX_PLAYER_X_VELOCITY / 2;
            mv.z = MAX_PLAYER_X_VELOCITY / 2;
            break;
        case MKeyDown:
            mv.x = 0.f;
            mv.z = MAX_PLAYER_X_VELOCITY;
            break;
        case MKeyDown | MKeyLeft:
            mv.x = -MAX_PLAYER_X_VELOCITY / 2;
            mv.z = MAX_PLAYER_X_VELOCITY / 2;
            break;
        case MKeyLeft:
            mv.x = -MAX_PLAYER_X_VELOCITY;
            mv.z = 0.f;
            break;
        case MKeyLeft | MKeyUp:
            mv.x = -MAX_PLAYER_X_VELOCITY / 2;
            mv.z = -MAX_PLAYER_X_VELOCITY / 2;
            break;
    }
    assert((abs(mv.x) + abs(mv.z)) <= MAX_PLAYER_X_VELOCITY);
}

void PlayerActor::handle_input(const SDL_Event& event)
{
    int lastMoveKeys = intentMoveKeys;
    switch (event.type) {
    case SDL_KEYDOWN:
        intentMoveKeys |= getMovementTypeFromKey(event.key.keysym.sym);
        intentKeys |= getIntentFromKey(event.key.keysym.sym);
        break;
    case SDL_KEYUP:
        intentMoveKeys &= ~getMovementTypeFromKey(event.key.keysym.sym);
        intentKeys &= ~getIntentFromKey(event.key.keysym.sym);
        break;
    case SDL_WINDOWEVENT:
        switch(event.window.type)
        {
            case SDL_WindowEventID::SDL_WINDOWEVENT_FOCUS_LOST:
                intentMove.x = intentMove.y = intentMove.z = 0.f;
                intentKeys = NoIntent;
                break;
        }
        break;
    }
    if(lastMoveKeys != intentMoveKeys) {
        if(intentMoveKeys != MKeyNone) {
            intentKeys |= Run;
            getMoveVectorFromMoveKey(intentMoveKeys, intentMove);
            // intentMove.angle = getMoveAngleFromMoveKey(intentMoveKeys);
            // intentMove.velocity = MAX_PLAYER_X_VELOCITY;
        } else {
            intentKeys &= ~Run;
            intentMove.x = 0.f;
            intentMove.z = 0.f;
        }
    }

    if (intentKeys & Jump) {
        intentMove.y = MAX_PLAYER_JUMP_VELOCITY;
    } else {
        intentMove.y = 0.f;
    }
}

/// <summary>
/// Move the actor horizontally
/// </summary>
/// <param name="isRunning">is the actor's intent currently Run</param>
/// <param name="deltaTime">time in seconds since last frame</param>
/// <param name="intent">The intended move vector (speed+angle)</param>
/// <param name="current">The current move vector (speed+angle)</param>
void Actor::handleMovement(float deltaTime, const MoveVector &intent, MoveVector &current)
{
    const CollisionData& colData = getCollisions();
    if (intent.x != current.x || intent.z != current.z) {

        // 0,1;90,0;180,-1;270;0,360,1

        float vDelta = PLAYER_RUN_ACCEL * deltaTime;
        float zDelta = intent.z - current.z;
        float xDelta = intent.x - current.x;
        if(zDelta == 0.f)
        {
            curMove.x += xDelta < 0? -vDelta : vDelta;
        } else if(xDelta == 0.f) {
            curMove.z += zDelta < 0? -vDelta : vDelta;
        } else {
            curMove.x += 0.5f * (xDelta < 0? -vDelta : vDelta);
            curMove.z += 0.5f * (zDelta < 0? -vDelta : vDelta);
        }
        if((curMove.x > 0.f && intent.x <= 0.f) || 
            (curMove.x < 0.f && intent.x >= 0.f)){
            curMove.x += (xDelta < 0 ? -vDelta : vDelta);
        }
        if((curMove.z > 0.f && intent.z <= 0.f) || 
            (curMove.z < 0.f && intent.z >= 0.f)){
            curMove.z += (zDelta < 0 ? -vDelta : vDelta);
        }
        if ((xDelta < 0 && curMove.x < intent.x) ||
            (xDelta > 0 && curMove.x > intent.x)) {
            curMove.x = intent.x;
        }
        if ((zDelta < 0 && curMove.z < intent.z) ||
            (zDelta > 0 && curMove.z > intent.z)) {
            curMove.z = intent.z;
        }

        if ((colData.directions & Left && curMove.x < 0.f) ||
            (colData.directions & Right && curMove.x > 0.f)) {
            curMove.x = 0.f;
        }
        if ((colData.directions & Front && curMove.z > 0.f) ||
            (colData.directions & Back && curMove.z < 0.f)) {
            curMove.z = 0.f; 
        }
    }
}

void Actor::handleCollisions()
{
    collisionGeomCurrent.x = realpos.x + collisionGeometry.x;
    collisionGeomCurrent.y1 = realpos.y + collisionGeometry.y1;
    collisionGeomCurrent.y2 = realpos.y + collisionGeometry.y2;
    collisionGeomCurrent.r = 0.5f;
    collisionGeomCurrent.z = realpos.z + 2.f;
    collisions = parentWindow.check_collision(collisionGeomCurrent);
}

void Actor::handleJump(float deltaTime)
{
        if(jumpEndTime == -1.f && intentMove.y != 0.f && state != ActorState::Hurt && state != ActorState::Jumping && getCollisions().directions & Down) {
            state = ActorState::Jumping;
            curMove.y = intentMove.y;
            jumpEndTime = maxJumpTime;
        } else if(jumpEndTime != -1.f) {
            jumpEndTime -= deltaTime;
            if (jumpEndTime <= 0 || intentMove.y == 0.f) {
                jumpEndTime = -1.f;
                intentMove.y = 0.f;
                state = ActorState::Default;
            }
        }
        if (collisions.directions & CollisionType::Up) {
            jumpEndTime = -1.f;
            state = ActorState::Default;
        }
}

void Actor::handleGravity(float deltaTime)
{
    if (jumpEndTime == -1.f) {
        if (collisions.directions & Down) {
            curMove.y = 0.0f;
            for (const auto& colItem : collisions.collisions) {
                if (colItem.direction & Down) {
                    realpos.y = colItem.surface.dimensions.p1.y;
                    break;
                }
            }
        } else {
            curMove.y -= gravity_accel * deltaTime;
            if (curMove.y < MIN_PLAYER_Y_VELOCITY) {
                curMove.y = MIN_PLAYER_Y_VELOCITY;
            }
        }
    }
}

/// <summary>
/// draw a frame of an Actor
/// </summary>
/// <param name="parentWindow">the window this actor belongs to</param>
/// <param name="deltaTime">the time (in seconds) elapsed since the last frame</param>
void Actor::draw(float deltaTime, const pixelpos& camera)
{
    const SDL_Rect& spriteRect = spriteProvider->getRect();
    if(collisionGeometry.x == -1.f) {
        collisionGeometry.x = float(spriteRect.x);
        collisionGeometry.y1 = spriteRect.y - 1.f;
        collisionGeometry.y2 = float(spriteRect.y);
        collisionGeometry.r = 0.5f;
        collisionGeometry.z = 2.f;
    }

    handleCollisions();
    handleMovement(deltaTime, intentMove, curMove);
    handleJump(deltaTime);
    handleGravity(deltaTime);
    realpos.y -= (curMove.y * deltaTime);
    realpos.z += (curMove.z * deltaTime);
    realpos.x += (curMove.x * deltaTime);
    if (visible) {
        getPixelPosFromRealPos(realpos, windowPos);
        texture->draw(spriteRect.x, spriteRect.y, windowPos.x - camera.x, windowPos.y - camera.y, spriteRect.w, spriteRect.h);
    }
}