#include "Actor.h"
#include "Geometry.h"

Actor::Actor(SpriteConfig* spriteConfig, Texture* texture, int mapX, int mapY) :
    spriteConfig(spriteConfig),
    texture(texture),
    x(-1),
    y(-1),
    z(-1),
    mapX(mapX),
    mapY(mapY),
    intentMoveAngle(0.0f),
    intentMovePercent(0.0f),
    curMoveVelocity(0.0f),
    curMoveAngle(0.0f),
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

void Actor::handle_input(const SDL_Event& event) 
{
    int lastMoveKeys = intentMoveKeys;
    switch (event.type) {
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_UP:
        case SDLK_w:
            intentMoveKeys |= MKeyUp;
            break;
        case SDLK_DOWN:
        case SDLK_s:
            intentMoveKeys |= MKeyDown;
            break;
        case SDLK_LEFT:
        case SDLK_a:
            intentMoveKeys |= MKeyLeft;
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            intentMoveKeys |= MKeyRight;
            if (!(intent & Attack)) {
                state = ActorState::Running;
            }
            break;
        case SDLK_LCTRL:
            intent |= Attack; 
            state = ActorState::Attacking;
            break;
        case SDLK_SPACE:
            intent |= Jump;
            break;
        }
        break;
    case SDL_KEYUP:
        switch (event.key.keysym.sym) {
        case SDLK_UP:
        case SDLK_w:
            intentMoveKeys &= ~MKeyUp;
            break;
        case SDLK_DOWN:
        case SDLK_s:
            intentMoveKeys &= ~MKeyDown;
            break;
        case SDLK_LEFT:
        case SDLK_a:
            intentMoveKeys &= ~MKeyLeft;
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            intentMoveKeys &= ~MKeyRight;
            break;
        case SDLK_LCTRL:
            intent &= Attack;
            break;
        case SDLK_SPACE:
            intent &= ~Jump;
            break;
        }
        break;
    }
    if(lastMoveKeys != intentMoveKeys) {
        if(intentMoveKeys != MKeyNone) {
            intent |= Run;
            switch(intentMoveKeys) {
                case MKeyUp:
                    intentMoveAngle = 0.f;
                    break;
                case MKeyUp | MKeyRight:
                    intentMoveAngle = 45.f;
                    break;
                case MKeyRight:
                    intentMoveAngle = 90.f;
                    break;
                case MKeyRight | MKeyDown:
                    intentMoveAngle = 135.f;
                    break;
                case MKeyDown:
                    intentMoveAngle = 180.f;
                    break;
                case MKeyDown | MKeyLeft:
                    intentMoveAngle = 225.f;
                    break;
                case MKeyLeft:
                    intentMoveAngle = 270.f;
                    break;
                case MKeyLeft | MKeyUp:
                    intentMoveAngle = 315.f;
                    break;
            }
            intentMovePercent = 1.f;
        } else {
            intent &= Run;
            intentMovePercent = 0.f;
            intentMoveAngle = 0.f;
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
    c.x = x + collisionGeometry.x;
    c.y1 = y + collisionGeometry.y - 1;
    c.y2 = y + collisionGeometry.y;
    c.r = 0.5f;
    c.z = z;
    for (const auto& wallGeometry : parentWindow->get_wall_geometries()) {
        cType |= get_collision(wallGeometry.dimensions, c);
    }
    return (CollisionType)cType;
}

// helper variable for determining the actual position of the actor given a z-offset and trying to determine the x- and y-offset
const double c_x_ratio = sqrt(5);

void Actor::draw(GameWindow* parentWindow, float deltaQuotient)
{
    if(x == -1 && y == -1 && z == -1) {
        tripoint actor_loc = parentWindow->getTripointAtMapPoint(mapX, mapY);
        x = actor_loc.x;
        y = actor_loc.y;
        z = actor_loc.z;
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
    if(intent & Run) {
        if(intentMoveAngle != curMoveAngle && curMoveVelocity != 0.f) {
            if (intentMoveAngle < curMoveAngle) {
                curMoveAngle += std::min(3.f, curMoveAngle - intentMoveAngle);
            }
            else {
                curMoveAngle -= std::min(3.f, intentMoveAngle - curMoveAngle);
            }
            curMoveVelocity -= (PLAYER_RUN_ACCEL * deltaQuotient) / 2;
        } else {
            curMoveAngle = intentMoveAngle;
            curMoveVelocity += (PLAYER_RUN_ACCEL * deltaQuotient);
        }
        if(curMoveVelocity > MAX_PLAYER_X_VELOCITY) {
            curMoveVelocity = MAX_PLAYER_X_VELOCITY;
        } else if (curMoveVelocity < 0.f) {
            curMoveVelocity = 0.f;
        }
    } else {
        if (curMoveVelocity > 0.f) {
            curMoveVelocity -= (PLAYER_RUN_ACCEL * deltaQuotient);
            if(curMoveVelocity <= 0.f) {
                curMoveVelocity = 0.f;
                curMoveAngle = 0.f;
            }
        }
    }
    if(curMoveVelocity != 0.0f) {
        // 0,1;90,0;180,-1;270;0,360,1
        float percentZ = abs(fmod(curMoveAngle, 360) - 180)/90-1;
        // 0,0;90,1;180,0;270,-1;360,0
        float percentX = 0;
        //if (curMoveAngle >= 0 && curMoveAngle <= 90) {
        //    percentX = curMoveAngle / 90;
        //} else {
            percentX = abs(fmod(curMoveAngle - 90, 360) - 180) / 90 - 1;
        //}
        float xmove = curMoveVelocity * percentX * deltaQuotient;
        if((colType & Left && xmove < 0.f) || 
            (colType & Right && xmove > 0.f)) {
                xmove = 0.f;
        }
        x += xmove;
        float zmove = curMoveVelocity * percentZ * deltaQuotient;
        if((colType & Front && zmove < 0.f) || 
            (colType & Back && zmove > 0.f)) {
                zmove = 0.f;
        }
        z += zmove;
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
        y -= y_velocity;
    }
    if (state == ActorState::Running) {
        spriteGroupIndex++;
        if (spriteGroupIndex == activeGroup->sprites.size()) {
            spriteGroupIndex = 0;
        }
    }

    int actualX = int((x * 16) + (z / c_x_ratio * 16));
    int actualY = int((z * 16 * 2 / c_x_ratio) + (y * 16));
    texture->draw(spriteRect.x, spriteRect.y, actualX, actualY, spriteRect.w, spriteRect.h);
}