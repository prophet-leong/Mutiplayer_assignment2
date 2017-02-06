#include "TimeBomb.h"
#include "hgeSprite.h"

TimeBomb::TimeBomb(float x, float y,float Radius, float time,int ID)
{
	this->x = x;
	this->y = y;
	this->Radius = Radius;
	this->time = time;
	this->ID = ID;
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load("bomb.png");
	hge->Release();
	sprite_.reset(new hgeSprite(tex_, 0, 0, 64, 64));
	sprite_->SetHotSpot(20, 10);
}
bool TimeBomb::Update(float dt)
{
	time -= dt;
	return(time < 0);
}
void TimeBomb::Render()
{
	sprite_->RenderEx(x, y, 0);
}
TimeBomb::~TimeBomb()
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Texture_Free(tex_);
	hge->Release();
}