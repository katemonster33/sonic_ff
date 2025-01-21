#include "Actor.h"

Actor::Actor(SpriteConfig* spriteConfigz, Texture *texture, size_t startx, size_t starty ) : 
    spriteConfig(spriteConfig),
    texture(texture),
    x(startx),
    y(starty),
    x_velocity(0.0f),
    y_velocity(0.0f),
    state(ActorState::Default),
    lastFrameState(ActorState::Default)
{
    visible = true;
    spriteGroupIndex = 0;
    activeGroup = nullptr;
}

Actor::~Actor()
{
    delete spriteConfig;
    delete texture;
}

void Actor::handle_input(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
        case SDLK_UP:
            intent &= ~MoveForward;
            intent |= MoveBack;
            state = ActorState::Running;
            break;
        case SDLK_DOWN:
            intent &= ~MoveBack;
            intent |= MoveForward;
            state = ActorState::Running;
            break;
        case SDLK_LEFT:
            intent &= ~MoveRight;
            intent |= MoveLeft;
            state = ActorState::Running;
            break;
        case SDLK_RIGHT:
            intent &= ~MoveLeft;
            intent |= MoveRight;
            state = ActorState::Running;
            break;
        }
        break;
    case SDL_KEYUP:
        switch (event.key.keysym.sym)
        {
        case SDLK_UP:
            intent &= ~MoveBack;
            break;
        case SDLK_DOWN: 
            intent &= ~MoveForward;
            break;
        case SDLK_LEFT:
            intent &= ~MoveLeft;
            break;
        case SDLK_RIGHT:
            intent &= ~MoveRight;
            break;
        }
        break;
    }
    if (activeGroup == nullptr || activeGroup->groupState != state)
    {
        for (const SpriteGroup& sg : spriteConfig->spriteGroups)
        {
            if (sg.groupState == state)
            {
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
    return CollisionType::None;
}

void Actor::draw(GameWindow* parentWindow, time_t frameTimeDelta)
{
    if (activeGroup == nullptr) {
        activeGroup = &spriteConfig->spriteGroups[0];
    }
    const SDL_Rect &spriteRect = activeGroup->sprites[spriteGroupIndex];
    texture->draw(spriteRect.x, spriteRect.y, x, y, spriteRect.w, spriteRect.h);
    CollisionType colType = check_collision(parentWindow);
    if (colType & CollisionType::Left || colType & CollisionType::Right)
    {
        x_velocity = 0.0f;
    }
    if (colType & CollisionType::Up || colType & CollisionType::Down)
    {
        y_velocity = z_velocity = 0;
    }
    x += x_velocity;
    if (x > int(parentWindow->GetSizeX() - spriteRect.w))
    {
        x = parentWindow->GetSizeX() - spriteRect.w;
    }
    else if (x < 0)
    {
        x = 0.0f;
    }
    if (y_velocity != 0.0f)
    {
        y += y_velocity;
        if (y > int(parentWindow->GetSizeY() - spriteRect.h))
        {
            y = parentWindow->GetSizeY() - spriteRect.h;
        }
        else if (y < 0)
        {
            y = 0.0f;
        }
    }
    if (state == ActorState::Running && x_velocity != 0.0f && y_velocity != 0.0f)
    {
        spriteGroupIndex++;
        if (spriteGroupIndex == activeGroup->sprites.size()) {
            spriteGroupIndex = 0;
        }
    }
}