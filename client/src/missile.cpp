#include <math.h>

#include "hge.h"
#include "hgeSprite.h"

#include "ship.h"
#include "missile.h"

Missile::Missile(char* filename, float x, float y, float w, int shipid ,int missileID) :
	angular_velocity(0)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load(filename);
	hge->Release();
	sprite_.reset(new hgeSprite(tex_, 0, 0, 40, 20));
	sprite_->SetHotSpot(20,10);
	x_ = x;
	y_ = y;
	w_ = w;
	ownerid = shipid;
	this->missileID = missileID;
	velocity_x_ = 200.0f * cosf(w_);
	velocity_y_ = 200.0f * sinf(w_); 

	x_ += velocity_x_ * 0.5f;
	y_ += velocity_y_ * 0.5f;
}

Missile::~Missile()
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Texture_Free(tex_);
	hge->Release();
}

Missile_Collided Missile::Update(std::vector<Ship*> &shiplist, float timedelta)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	float pi = 3.141592654f*2;
	//float oldx, oldy;
	Missile_Collided collide;
	w_ += angular_velocity * timedelta;
	if (w_ > pi)
		w_ -= pi;

	if (w_ < 0.0f)
		w_ += pi;

	//oldx = x_;
	//oldy = y_;
	x_ += velocity_x_ * timedelta;
	y_ += velocity_y_ * timedelta;
	for (int i = 0; i < shiplist.size(); ++i)
	{
		if (shiplist[i] && HasCollided(*shiplist[i]))
		{
			collide.collisionType = 1;
			collide.ShipID = shiplist[i]->GetID();
			--shiplist[i]->health;
			if (shiplist[i]->health <= 0)
			{
				std::swap(shiplist[i], shiplist[shiplist.size() - 1]);
				Ship*temp = shiplist[shiplist.size() - 1];
				shiplist.pop_back();
				delete temp;
			}
			return collide;
		}
	}
	
	float screenwidth = static_cast<float>(hge->System_GetState(HGE_SCREENWIDTH));
	float screenheight = static_cast<float>(hge->System_GetState(HGE_SCREENHEIGHT));
	float spritewidth = sprite_->GetWidth();
	float spriteheight = sprite_->GetHeight();
	
	if (x_ < spritewidth / 2)
	{
		collide.collisionType = 2;
		return collide;
	}
	else if (x_ > screenwidth - spritewidth/2)
	{
		collide.collisionType = 2;
		return collide;
	}
	if (y_ < spriteheight/2)
	{
		collide.collisionType = 2;
		return collide;
	}
	else if (y_ > screenheight - spriteheight/2)
	{
		collide.collisionType = 2;
		return collide;
	}
	return collide;
}

void Missile::Render()
{
	sprite_->RenderEx(x_, y_, w_);
}

bool Missile::HasCollided( Ship &ship )
{
	sprite_->GetBoundingBox( x_, y_, &collidebox);

	return collidebox.Intersect( ship.GetBoundingBox() );
}