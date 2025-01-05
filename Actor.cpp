#include "Actor.h"

Actor::Actor( SpriteSheet* sheet )
{
	this->sheet = sheet;
	state = ActorState::Default;
}