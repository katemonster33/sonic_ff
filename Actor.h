#include "SpriteSheet.h"
#include "ActorState.h"

struct Bounds
{
	int x;
	int y;
	int w;
	int h;
};

class Actor
{
	ActorState state;
	SpriteSheet *sheet;
	Bounds bounds;
	bool visible;
public:
	Actor( SpriteSheet* sheet );

	ActorState GetState() { return state; }
};