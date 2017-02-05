#include "PowerUp.h"


PowerUp::PowerUp(float x, float y, float vel_x, float vel_y,int ID)
{
	this->ID = ID;
	this->x = x;
	this->y = y;
	this->vel_x = vel_x;
	this->vel_y = vel_y;
}
void PowerUp::Update(float dt)
{
	x += vel_x*dt;
	y += vel_y*dt;
}
PowerUp::~PowerUp()
{
}
