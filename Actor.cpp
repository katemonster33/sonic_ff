#include "Actor.h"
#include "SpriteSheet.h"
#include "Geometry.h"

Actor::Actor(SpriteConfig* spriteConfig, Texture* texture, const mappoint &mt) :
    spriteConfig(spriteConfig),
    texture(texture),
    mappos(mt),
    realpos({-1.f, -1.f, -1.f}),
    intentMove( MoveVector{0.f, 0.f} ),
    curMove( MoveVector{0.f, 0.f} ),
    y_velocity(0.0f),
    collisionGeometry({-1.f, -1.f, -1.f, -1.f}),
    state(ActorState::Default),
    lastFrameState(ActorState::Default),
    intent(NoIntent),
    intentMoveKeys(MKeyNone),
    visible(true),
    spriteGroupIndex(0),
    activeGroup(nullptr)
{
}

Actor::~Actor()
{
    delete spriteConfig;
    delete texture;
}

bool Actor::isMovementKey(SDL_Keycode keyCode)
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

int Actor::getMovementTypeFromKey(SDL_Keycode keyCode)
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

int Actor::getIntentFromKey(SDL_Keycode keyCode)
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

float Actor::getMoveAngleFromMoveKey(int mKeysDown)
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

void Actor::handle_input(const SDL_Event& event) 
{
    int lastMoveKeys = intentMoveKeys;
    switch (event.type) {
    case SDL_KEYDOWN:
        if(isMovementKey(event.key.keysym.sym)) {
            intent |= Run;
            intentMoveKeys |= getMovementTypeFromKey(event.key.keysym.sym);
        } else {
            intent |= getIntentFromKey(event.key.keysym.sym);
        }
        break;
    case SDL_KEYUP:
        if(isMovementKey(event.key.keysym.sym)) {
            intent &= ~Run;
            intentMoveKeys &= ~getMovementTypeFromKey(event.key.keysym.sym);
        } else {
            intent &= ~getIntentFromKey(event.key.keysym.sym);
        }
        break;
    }
    if(lastMoveKeys != intentMoveKeys) {
        if(intentMoveKeys != MKeyNone) {
            intent |= Run;
            intentMove.angle = getMoveAngleFromMoveKey(intentMoveKeys);
            intentMove.velocity = MAX_PLAYER_X_VELOCITY;
        } else {
            intent &= ~Run;
            intentMove.velocity = 0.f;
            intentMove.angle = 0.f;
        }
    }
    if (activeGroup == nullptr || activeGroup->groupState != state) {
        for (const SpriteGroup& sg : spriteConfig->spriteGroups) {
            if (sg.groupState == state) {
                activeGroup = &sg;
            }
        }
        if (activeGroup == nullptr) {
            activeGroup = &spriteConfig->spriteGroups[0];
        }
    }
}

CollisionType Actor::check_collision(GameWindow* parentWindow)
{
    int cType = CollisionType::NoCollision;
    cylinder c;
    c.x = realpos.x + collisionGeometry.x;
    c.y1 = realpos.y + collisionGeometry.y - 1;
    c.y2 = realpos.y + collisionGeometry.y;
    c.r = 0.5f;
    c.z = realpos.z;
    for (const auto& wallGeometry : parentWindow->get_wall_geometries()) {
        cType |= get_collision(wallGeometry.dimensions, c);
    }
    for (const auto& ground : parentWindow->get_ground_geometries()) {
        cType |= get_collision(ground.dimensions, c);
    }
    return (CollisionType)cType;
}

void Actor::handleMovement(bool isRunning, float deltaTime, const MoveVector &intent, MoveVector &current)
{
    if(isRunning) {
        if(intent.angle != current.angle && current.velocity != 0.f) {
            if (intent.angle == abs(current.angle - 180)) {
                current.velocity -= (PLAYER_RUN_ACCEL * deltaTime);
            } else {
                if (fmod(intent.angle + 180, 360) < current.angle) {
                    current.angle -= std::min(3.f, current.angle - intent.angle);
                }
                else {
                    current.angle += std::min(3.f, intent.angle - current.angle);
                }
                current.velocity -= (PLAYER_RUN_ACCEL * deltaTime) / 2;
            }
            
        } else {
            current.angle = intent.angle;
            current.velocity += (PLAYER_RUN_ACCEL * deltaTime);
        }
        if(current.velocity > MAX_PLAYER_X_VELOCITY) {
            current.velocity = MAX_PLAYER_X_VELOCITY;
        } else if (current.velocity < 0.f) {
            current.velocity = 0.f;
        }
    } else {
        if (current.velocity > 0.f) {
            current.velocity -= (PLAYER_RUN_ACCEL * deltaTime);
            if(current.velocity <= 0.f) {
                current.velocity = 0.f;
                current.angle = 0.f;
            }
        }
    }
}

void Actor::draw(GameWindow* parentWindow, float deltaQuotient)
{
    if(realpos.x == -1 && realpos.y == -1 && realpos.z == -1) {
        tripoint actor_loc = parentWindow->getTripointAtMapPoint(mappos);
        realpos.x = actor_loc.x;
        realpos.y = actor_loc.y;
        realpos.z = actor_loc.z;
    }
    if (activeGroup == nullptr) {
        activeGroup = &spriteConfig->spriteGroups[0];
    }
    const SDL_Rect &spriteRect = activeGroup->sprites[spriteGroupIndex];
    if(collisionGeometry.x == -1.f) {
        collisionGeometry.x = collisionGeometry.y = 0;
        collisionGeometry.w = spriteRect.w;
        collisionGeometry.h = spriteRect.h;
    }
    CollisionType colType = check_collision(parentWindow);
    handleMovement(intent & Run, deltaQuotient, intentMove, curMove);
    if(curMove.velocity != 0.0f) {
        // 0,1;90,0;180,-1;270;0,360,1
        float percentZ = -(abs(fmod(curMove.angle, 360) - 180)/90-1);
        // 0,0;90,1;180,0;270,-1;360,0
        float percentX = abs(fmod(curMove.angle + 270, 360) - 180) / 90 - 1;
        float xmove = curMove.velocity * percentX * deltaQuotient;
        if((colType & Left && xmove < 0.f) || 
            (colType & Right && xmove > 0.f)) {
                xmove = 0.f;
        }
        realpos.x += xmove;
        float zmove = curMove.velocity * percentZ * deltaQuotient;
        if((colType & Front && zmove > 0.f) || 
            (colType & Back && zmove < 0.f)) {
                zmove = 0.f;
        }
        realpos.z += zmove;
    }

    if (intent & Jump) {
        if(colType & Down) {
            y_velocity = MAX_PLAYER_JUMP_VELOCITY;
        }
    }
    if(!(colType & Down)) {
        y_velocity -= gravity_accel * deltaQuotient;
        if(y_velocity < MIN_PLAYER_Y_VELOCITY) {
            y_velocity = MIN_PLAYER_Y_VELOCITY;
        }
        realpos.y -= y_velocity;
    }
    if (state == ActorState::Running) {
        spriteGroupIndex++;
        if (spriteGroupIndex == activeGroup->sprites.size()) {
            spriteGroupIndex = 0;
        }
    }

    int actualX = 0, actualY = 0;
    getPixelPosFromRealPos(realpos, actualX, actualY);
    texture->draw(spriteRect.x, spriteRect.y, int(actualX), int(actualY), spriteRect.w, spriteRect.h);
}