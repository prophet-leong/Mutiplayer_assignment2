#include "PowerUp.h"

#include "hge.h"
#include "hgeSprite.h"
#include "hgeFont.h"
#include "ship.h"

PowerUp::PowerUp(float x, float y, float vel_x, float vel_y,int ID)
{
	this->ID = ID;
	this->x = x;
	this->y = y;
	this->vel_x = vel_x;
	this->vel_y = vel_y;
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load("boom.png");

	hge->Release();
	sprite_.reset(new hgeSprite(tex_, 0, 0, 64, 64));
}
bool PowerUp::Update(float dt,Ship* ship)
{
	x += vel_x*dt;
	y += vel_y*dt;

	return ((x - ship->GetX())*(x - ship->GetX()) + (y - ship->GetY())*(y - ship->GetY()) < 100.f);
}
void PowerUp::Render()
{
	sprite_->RenderEx(x, y, 0);
}
PowerUp::~PowerUp()
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Texture_Free(tex_);
	hge->Release();
	sprite_.release();
}
