#include "SpriteSheet.h"

struct Bounds
{
	int x;
	int y;
	int w;
	int h;
};

class Actor
{
	SpriteSheet *sheet;
	Bounds bounds;
	bool visible;
public:
	Actor( SpriteSheet* sheet );
};