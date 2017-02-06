#include "RespawnGate.h"
#include "hge.h"
#include "hgeSprite.h"

RespawnGate::RespawnGate(float x,float y)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Release();
	tex_ = hge->Texture_Load("gate.png");
	sprite_.reset(new hgeSprite(tex_, 0, 0, 64, 64));
	sprite_->SetHotSpot(32, 32);
	this->x = x;
	this->y = y;
}

RespawnGate::~RespawnGate()
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Texture_Free(tex_);
	hge->Release();
	sprite_.release();
}
void RespawnGate::SetNewGate(float x, float y)
{
	this->x = x;
	this->y = y;
}
float RespawnGate::GetX()
{
	return x;
}
float RespawnGate::GetY()
{
	return y;
}
void RespawnGate::Render()
{
	sprite_->RenderEx(x, y, 0);
}