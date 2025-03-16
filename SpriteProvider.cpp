#include "SpriteSheet.h"

SpriteProvider::SpriteProvider(SpriteConfig& spriteConfig) :
    spriteConfig(spriteConfig),
    spriteGroupIndex(0),
    activeGroup(&spriteConfig.spriteGroups[0]),
    spriteRect({0, 0})
{
}

void SpriteProvider::update(ActorState currentState)
{
    if (activeGroup->groupState != currentState) {
        for (const SpriteGroup& sg : spriteConfig.spriteGroups) {
            if (sg.groupState == currentState) {
                activeGroup = &sg;
            }
        }
    }
    if (currentState == ActorState::Running) {
        spriteGroupIndex++;
        if (spriteGroupIndex == activeGroup->sprites.size()) {
            spriteGroupIndex = 0;
        }
    }
}

void SpriteProvider::draw()
{

}