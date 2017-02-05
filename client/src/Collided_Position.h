#pragma once

#include "hge.h"
#include "hgerect.h"
#include <memory>
#include <vector>

class hgeSprite;
class hgeFont;

class Collided_Position
{
public:
	Collided_Position(float x,float y,float timer);
	~Collided_Position();
	bool Update(float dt);
	void Render();
	float x;
	float y;
	float timer;
private:
	HTEXTURE tex_; //!< Handle to the sprite's texture
	std::auto_ptr<hgeSprite> sprite_; //!< The sprite used to display the ship
};

