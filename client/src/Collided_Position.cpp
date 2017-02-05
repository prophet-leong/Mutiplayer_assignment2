#include "Collided_Position.h"
#include "hge.h"
#include "hgeSprite.h"
#include "hgeFont.h"

Collided_Position::Collided_Position(float x_, float y_, float timer_)
:x(x_),
y(y_),
timer(timer_)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load("boom.png");

	hge->Release();
	sprite_.reset(new hgeSprite(tex_, 0, 0, 40, 40));
}

bool Collided_Position::Update(float dt)
{
	timer -= dt;
	return (timer < 0);
}
void Collided_Position::Render()
{
	sprite_->RenderEx(x, y, 0);
}
Collided_Position::~Collided_Position()
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Texture_Free(tex_);
	hge->Release();
	sprite_.release();
}
