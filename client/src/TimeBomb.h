#pragma once
#include <vector>
#include <memory>
#include <vector>

#include "hge.h"


class hgeSprite;
class TimeBomb
{
public:

	HTEXTURE tex_; //!< Handle to the sprite's texture
	std::auto_ptr<hgeSprite> sprite_; //!< The sprite used to display the ship

	TimeBomb(float x,float y,float Radius,float time,int ID);
	~TimeBomb();
	void Render();
	bool Update(float dt);
	float x;
	float y;
	float time;
	float Radius;
	int ID;
private:
};

