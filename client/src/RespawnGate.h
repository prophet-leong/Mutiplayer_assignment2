#pragma once
#include <memory>

#include "hge.h"

class hgeSprite;
class hgeFont;


class RespawnGate
{
public:
	RespawnGate(float x,float y);
	~RespawnGate();
	HTEXTURE tex_; //!< Handle to the sprite's texture
	std::auto_ptr<hgeSprite> sprite_; //!< The sprite used to display the ship
	void SetNewGate(float x, float y);
	float GetX();
	float GetY();
	void Render();
private:
	float x;
	float y;
};