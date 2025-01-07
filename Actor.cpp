#include "Actor.h"

Actor::Actor(SpriteConfig* spriteConfig, Texture *texture, size_t x, size_t y ) : 
    spriteConfig(spriteConfig),
    texture(texture),
    x(x),
    y(y)
{
	state = ActorState::Default;
    visible = true;
    spriteGroupIndex = 0;
    activeGroup = nullptr;
}

Actor::~Actor()
{
    delete spriteConfig;
    delete texture;
}

void Actor::handle_input(SDL_Event* event)
{
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

void Actor::draw(GameWindow* parentWindow)
{
    if (activeGroup == nullptr) {
        activeGroup = &spriteConfig->spriteGroups[0];
    }
    const SDL_Rect &spriteRect = activeGroup->sprites[spriteGroupIndex];
    texture->draw(spriteRect.x, spriteRect.y, 0, 0, spriteRect.w, spriteRect.h);
}