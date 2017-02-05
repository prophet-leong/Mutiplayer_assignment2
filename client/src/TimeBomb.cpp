#include "TimeBomb.h"


TimeBomb::TimeBomb(float x, float y,float Radius, float time,int ID)
{
	this->x = x;
	this->y = y;
	this->Radius = Radius;
	this->time = time;
	this->ID = ID;
}
bool TimeBomb::Update(float dt)
{
	time -= dt;
	return(time < 0);
}
TimeBomb::~TimeBomb()
{
}