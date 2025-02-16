#include "Actor.h"
#include "Geometry.h"

Actor::Actor(SpriteConfig* spriteConfig, Texture* texture, int startx, int starty, int startz) :
    spriteConfig(spriteConfig),
    texture(texture),
    x(startx),
    y(starty),
    z(startz),
    x_velocity(0.0f),
    y_velocity(0.0f),
    z_velocity(0.0f),
    height(-1),
    jump_height(0),
    jump_velocity(0.0f),
    state(ActorState::Default),
    lastFrameState(ActorState::Default),
    intent(NoIntent),
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
    switch (event.type) {
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_UP:
        case SDLK_w:
            intent &= ~MoveForward;
            intent |= MoveBack;
            if (!(intent & Attack)) {
                state = ActorState::Running;
            }
            break;
        case SDLK_DOWN:
        case SDLK_s:
            intent &= ~MoveBack;
            intent |= MoveForward;
            if (!(intent & Attack)) {
                state = ActorState::Running;
            }
            break;
        case SDLK_LEFT:
        case SDLK_a:
            intent &= ~MoveRight;
            intent |= MoveLeft;
            if (!(intent & Attack)) {
                state = ActorState::Running;
            }
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            intent &= ~MoveLeft;
            intent |= MoveRight;
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
            intent &= ~MoveBack;
            break;
        case SDLK_DOWN:
        case SDLK_s:
            intent &= ~MoveForward;
            break;
        case SDLK_LEFT:
        case SDLK_a:
            intent &= ~MoveLeft;
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            intent &= ~MoveRight;
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
    c.y1 = y + collisionGeometry.y;
    c.y2 = y + collisionGeometry.y + collisionGeometry.h;
    c.r = collisionGeometry.w / 2;
    c.z = z;
    for (const auto& wallGeometry : parentWindow->get_wall_geometries()) {
        cType |= get_collision(wallGeometry.dimensions, c);
    }
    return (CollisionType)cType;
}

// helper variable for determining the actual position of the actor given a z-offset and trying to determine the x- and y-offset
const double c_x_ratio = sqrt(5);

void Actor::draw(GameWindow* parentWindow, uint64_t frameTimeDelta)
{
    if (height == -1) {
        height = parentWindow->getHeight(x, y);
    }
    if (activeGroup == nullptr) {
        activeGroup = &spriteConfig->spriteGroups[0];
    }
    const SDL_Rect &spriteRect = activeGroup->sprites[spriteGroupIndex];
    int actualX = x + int(z / c_x_ratio);
    int actualY = int(z * 2 / c_x_ratio) - jump_height - y;
    texture->draw(spriteRect.x, spriteRect.y, actualX, actualY, spriteRect.w, spriteRect.h);
    CollisionType colType = check_collision(parentWindow);
    if (colType == NoCollision || colType == Down) {
        if (intent & MoveLeft) {
            x_velocity -= 0.1f;
            if (x_velocity < -5.0f) {
                x_velocity = -5.0f;
            }
        }
        else if (x_velocity < -0.1f) {
            x_velocity += 0.1f;
        }
        if (intent & MoveRight) {
            x_velocity += 0.1f;
            if (x_velocity > 5.0f) {
                x_velocity = 5.0f;
            }
        } else if (x_velocity > 0.1f) {
            x_velocity -= 0.1f;
        }
        if (intent & MoveBack) {
            z_velocity -= 0.1f;
            if (z_velocity < -1.0f) {
                z_velocity = -1.0f;
            }
        } else if(z_velocity < -0.1f) {
            z_velocity += 0.1f;
        }
        if (intent & MoveForward) {
            z_velocity += 0.1f;
            if (z_velocity > 1.0f) {
                z_velocity = 1.0f;
            }
        } else if(z_velocity > 0.1f) {
            z_velocity -= 0.1f;
        }
    }
    else {
        if (colType & CollisionType::Left || colType & CollisionType::Right) {
            x_velocity = 0.0f;
        }
        if (colType & CollisionType::Up || colType & CollisionType::Down) {
            y_velocity = 0;
        }
    }

    if (intent & Jump) {
        if (jump_height < 3) {
            if (jump_velocity < 10.0f) {
                jump_velocity += 1.0f;
            }
        }
        else if (jump_height >= 8) {
            jump_velocity -= 0.4f;
        }
        else if (y <= 0 && jump_velocity <= 0.0f) {
            jump_height = 0;
            jump_velocity = 0.0f;
        }
    }
    else if (jump_velocity > 0.0f) {
        if (colType & CollisionType::Down) {
            y += jump_height;
            jump_height = 0;
            jump_velocity = 0.0f;
        }
        else if (jump_velocity > -10.0f) {
            jump_velocity -= 0.4f;
        }
    }
    jump_height += jump_velocity;
    x += x_velocity;
    if (x > int(parentWindow->GetSizeX() - spriteRect.w)) {
        x = parentWindow->GetSizeX() - spriteRect.w;
    }
    else if (x < 0) {
        x = 0;
    }
    z += z_velocity;
    if (z < 0) {
        z = 0;
    }
    if (y_velocity != 0.0f) {
        y += y_velocity;
        if (y > int(parentWindow->GetSizeY() - spriteRect.h)) {
            y = parentWindow->GetSizeY() - spriteRect.h;
        } else if (y < 0) {
            y = 0.0f;
        }
    }
    if (state == ActorState::Running && x_velocity != 0.0f && y_velocity != 0.0f) {
        spriteGroupIndex++;
        if (spriteGroupIndex == activeGroup->sprites.size()) {
            spriteGroupIndex = 0;
        }
    }
}