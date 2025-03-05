#include "Actor.h"
#include "SpriteSheet.h"
#include "Geometry.h"
#include "GameWindow.h"

Actor::Actor(GameWindow& parentWindow, SpriteConfig* spriteConfig, Texture* texture, const mappoint &mt) :
    spriteConfig(spriteConfig),
    texture(texture),
    parentWindow(parentWindow),
    realpos(parentWindow.getTripointAtMapPoint(mt)),
    intentMove( MoveVector{0.f, 0.f} ),
    curMove( MoveVector{0.f, 0.f} ),
    y_velocity(0.0f),
    collisionGeometry({-1.f, -1.f, -1.f, -1.f}),
    spriteRect({-1, -1, -1, -1}),
    state(ActorState::Default),
    lastFrameState(ActorState::Default),
    visible(true),
    spriteGroupIndex(0),
    activeGroup(spriteConfig->spriteGroups[0]),
    maxJumpTime(DEFAULT_JUMP_TIME),
    jumpEndTime(-1.f),
    collisions({0}),
    collisionGeomCurrent({ -1.f, -1.f, -1.f, -1.f, -1.f })
{
}

Actor::~Actor()
{
}

PlayerActor::PlayerActor(GameWindow& parentWindow, SpriteConfig* spriteConfig, Texture* texture, const mappoint& mt) : 
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

float PlayerActor::getMoveAngleFromMoveKey(int mKeysDown)
{
    switch(mKeysDown) {
        case MKeyUp:
        default:
            return 0.f;
        case MKeyUp | MKeyRight:
            return 45.f;
        case MKeyRight:
            return 90.f;
        case MKeyRight | MKeyDown:
            return 135.f;
        case MKeyDown:
            return 180.f;
        case MKeyDown | MKeyLeft:
            return 225.f;
        case MKeyLeft:
            return 270.f;
        case MKeyLeft | MKeyUp:
            return 315.f;
    }
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
    }
    if(lastMoveKeys != intentMoveKeys) {
        if(intentMoveKeys != MKeyNone) {
            intentKeys |= Run;
            intentMove.angle = getMoveAngleFromMoveKey(intentMoveKeys);
            intentMove.velocity = MAX_PLAYER_X_VELOCITY;
        } else {
            intentKeys &= ~Run;
            intentMove.velocity = 0.f;
            intentMove.angle = 0.f;
        }
    }
    if (activeGroup.groupState != state) {
        for (const SpriteGroup& sg : spriteConfig->spriteGroups) {
            if (sg.groupState == state) {
                activeGroup = sg;
            }
        }
    }

    if (intentKeys & Jump && getCollisions().directions & Down && state != ActorState::Hurt) {
        state = ActorState::Jumping;
    } else if (state == ActorState::Jumping && intentKeys != Jump) {
        state = ActorState::Default;
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

/// <summary>
/// Move the actor horizontally
/// </summary>
/// <param name="isRunning">is the actor's intent currently Run</param>
/// <param name="deltaTime">time in seconds since last frame</param>
/// <param name="intent">The intended move vector (speed+angle)</param>
/// <param name="current">The current move vector (speed+angle)</param>
void PlayerActor::handleMovement(float deltaTime, const MoveVector& intent, MoveVector& current)
{
    const CollisionData& colData = getCollisions();
    if (intentKeys & Run) {
        if (intent.angle != current.angle && current.velocity != 0.f) {
            if (intent.angle == abs(current.angle - 180) || intent.angle == abs(current.angle + 180)) {
                current.velocity -= (PLAYER_RUN_ACCEL * deltaTime * 2);
            }
            else {
                modifyVelocityFromTurn(intent, current, PLAYER_RUN_ACCEL * deltaTime);
            }
        } else {
            current.angle = intent.angle;
            current.velocity += (PLAYER_RUN_ACCEL * deltaTime);
        }
        if (current.velocity > MAX_PLAYER_X_VELOCITY) {
            current.velocity = MAX_PLAYER_X_VELOCITY;
        } else if (current.velocity < 0.f) {
            current.velocity = 0.f;
        }
    }
    else {
        if (current.velocity > 0.f) {
            current.velocity -= (PLAYER_RUN_ACCEL * deltaTime);
            if (current.velocity <= 0.f) {
                current.velocity = 0.f;
                current.angle = 0.f;
            }
        }
    }
    if (curMove.velocity != 0.0f) {
        // 0,1;90,0;180,-1;270;0,360,1
        float percentZ = -(fabs(float(fmod(curMove.angle, 360) - 180)) / 90 - 1);
        // 0,0;90,1;180,0;270,-1;360,0
        float percentX = abs(float(fmod(curMove.angle + 270, 360) - 180)) / 90 - 1;
        float xmove = curMove.velocity * percentX * deltaTime;
        if ((colData.directions & Left && xmove < 0.f) ||
            (colData.directions & Right && xmove > 0.f)) {
            xmove = 0.f;
        }
        realpos.x += xmove;
        float zmove = curMove.velocity * percentZ * deltaTime;
        if ((colData.directions & Front && zmove > 0.f) ||
            (colData.directions & Back && zmove < 0.f)) {
            zmove = 0.f;
        }
        realpos.z += zmove;
    }
}

void Actor::handleJump(float deltaTime)
{
    if (state == ActorState::Jumping) {
        if(jumpEndTime == -1.f) {
            y_velocity = MAX_PLAYER_JUMP_VELOCITY;
            jumpEndTime = maxJumpTime;
        } else {
            jumpEndTime -= deltaTime;
            if (jumpEndTime <= 0) {
                jumpEndTime = -1.f;
                state = ActorState::Default;
            }
        }
        if (collisions.directions & CollisionType::Up) {
            jumpEndTime = -1.f;
            state = ActorState::Default;
        }
    }
}

void Actor::handleGravity(float deltaTime)
{
    if (state != ActorState::Jumping) {
        if (collisions.directions & Down) {
            y_velocity = 0.0f;
            for (const auto& colItem : collisions.collisions) {
                if (colItem.direction & Down) {
                    realpos.y = colItem.surface.dimensions.p1.y;
                    break;
                }
            }
        } else {
            y_velocity -= gravity_accel * deltaTime;
            if (y_velocity < MIN_PLAYER_Y_VELOCITY) {
                y_velocity = MIN_PLAYER_Y_VELOCITY;
            }
        }
    }
}

/// <summary>
/// draw a frame of an Actor
/// </summary>
/// <param name="parentWindow">the window this actor belongs to</param>
/// <param name="deltaTime">the time (in seconds) elapsed since the last frame</param>
void Actor::draw(float deltaTime)
{
    const SDL_Rect &spriteRect = activeGroup.sprites[spriteGroupIndex];
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
    realpos.y -= y_velocity;
    if (state == ActorState::Running) {
        spriteGroupIndex++;
        if (spriteGroupIndex == activeGroup.sprites.size()) {
            spriteGroupIndex = 0;
        }
    }
    if (visible) {
        int actualX = 0, actualY = 0;
        getPixelPosFromRealPos(realpos, actualX, actualY);
        texture->draw(spriteRect.x, spriteRect.y, int(actualX), int(actualY), spriteRect.w, spriteRect.h);
    }
}