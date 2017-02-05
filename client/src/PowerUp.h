#pragma once
#ifndef POWERUP_H
#define POWERUP_H

#include "hge.h"
#include "hgerect.h"
#include <memory>
#include <vector>


class Ship;
class hgeSprite;
class hgeFont;
class hgeRect;

class PowerUp
{
public:
	PowerUp(float x,float y,float vel_x,float vel_y,int ID);
	~PowerUp();
	bool Update(float dt,Ship*myship);
	void Render();
	hgeRect* GetBoundingBox();
	int ID;
private:
	HTEXTURE tex_; //!< Handle to the sprite's texture
	std::auto_ptr<hgeSprite> sprite_; //!< The sprite used to display the ship
	hgeRect collidebox;
	float x;
	float y;
	float vel_x;
	float vel_y;
};

#endif // !POWERUP_H

