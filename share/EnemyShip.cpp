#include"EnemyShip.h"
#include "hge.h"
#include "hgeSprite.h"
#include "hgeFont.h"

const float PI = 3.141592f;

Ship::Ship(int type, float locx_, float locy_)
: w_(-PI / 2)
, angular_velocity(0)
, velocity_x_(0)
, velocity_y_(0)
, id(0)
, collidetimer(0)
#ifdef INTERPOLATEMOVEMENT
, server_w_(-PI / 2)
, client_w_(-PI / 2)
, server_velx_(0)
, server_vely_(0)
, ratio_(1)
#endif
{
	std::cout << "Creating Ship " << type << " " << locx_ << " " << locy_ << std::endl;
#ifdef INTERPOLATEMOVEMENT
	x_ = server_x_ = client_x_ = locx_;
	y_ = server_y_ = client_y_ = locy_;
#else
	x_ = locx_;
	y_ = locy_;
#endif

	HGE* hge = hgeCreate(HGE_VERSION);

	switch (type)
	{
	case 2:
		tex_ = hge->Texture_Load(SHIPTYPE2);
		type_ = 2;
		break;
	case 3:
		tex_ = hge->Texture_Load(SHIPTYPE3);
		type_ = 3;
		break;
	case 4:
		tex_ = hge->Texture_Load(SHIPTYPE4);
		type_ = 4;
		break;
	default:
		tex_ = hge->Texture_Load(SHIPTYPE1);
		type_ = 1;
		break;
	}

	hge->Release();
	sprite_.reset(new hgeSprite(tex_, 0, 0, 64, 64));

	font_.reset(new hgeFont("font1.fnt"));
	font_->SetScale(0.5);
	sprite_->SetHotSpot(32, 32);

	/*
	x_ = server_x_ = client_x_ = locx_;
	y_ = server_y_ = client_y_ = locy_;
	HGE* hge = hgeCreate(HGE_VERSION);
	tex_ = hge->Texture_Load(filename);
	hge->Release();
	sprite_.reset(new hgeSprite(tex_, 0, 0, 64, 64));

	font_.reset(new hgeFont("font1.fnt"));
	font_->SetScale( 0.5 );
	sprite_->SetHotSpot(32,32);
	*/
}


/**
* Ship Destructor
*
* Frees the internal texture used by the sprite
*/
Ship::~Ship()
{
	HGE* hge = hgeCreate(HGE_VERSION);
	hge->Texture_Free(tex_);
	hge->Release();
	sprite_.release();
	font_.release();
}


/**
* Update cycle
*
* Increments the angular and x,y position of the ship based on how
* much time that has passed since the last frame. It also wraps the
* ship around the screen so it never goes out of screen.
*
* @param timedelta The time that has passed since the last frame in milliseconds
*/

void Ship::Update(float timedelta)
{
	HGE* hge = hgeCreate(HGE_VERSION);
	float pi = 3.141592654f * 2;

	server_w_ += angular_velocity * timedelta;

	if (server_w_ > pi)
		server_w_ -= pi;

	if (server_w_ < 0.0f)
		server_w_ += pi;

	client_w_ += angular_velocity * timedelta;

	if (client_w_ > pi)
		client_w_ -= pi;

	if (client_w_ < 0.0f)
		client_w_ += pi;

	w_ = ratio_ * server_w_ + (1 - ratio_) * client_w_;

	if (w_ > pi)
		w_ -= pi;

	if (w_ < 0.0f)
		w_ += pi;

	float screenwidth = static_cast<float>(hge->System_GetState(HGE_SCREENWIDTH));
	float screenheight = static_cast<float>(hge->System_GetState(HGE_SCREENHEIGHT));
	float spritewidth = sprite_->GetWidth();
	float spriteheight = sprite_->GetHeight();


	server_x_ += server_velx_ * timedelta;
	server_y_ += server_vely_ * timedelta;

	// Deadreckon
	if (server_x_ < spritewidth / 2)
	{
		server_x_ = spritewidth / 2 + 5;
		server_velx_ = 0;
	}
	else if (server_x_ > screenwidth - spritewidth / 2)
	{
		server_x_ = screenwidth - spritewidth / 2 - 5;
		server_velx_ = 0;
	}

	if (server_y_ < spriteheight / 2)
	{
		server_y_ = spriteheight / 2 + 5;
		server_vely_ = 0;
	}
	else if (server_y_ > screenheight - spriteheight / 2)
	{
		server_y_ = screenheight - spriteheight / 2 - 5;
		server_vely_ = 0;
	}


	client_x_ += velocity_x_ * timedelta;
	client_y_ += velocity_y_ * timedelta;

	// Deadreckon
	if (client_x_ < spritewidth / 2)
		client_x_ = spritewidth / 2;
	else if (client_x_ > screenwidth - spritewidth / 2)
		client_x_ = screenwidth - spritewidth / 2;

	if (client_y_ < spriteheight / 2)
		client_y_ = spriteheight / 2;
	else if (client_y_ > screenheight - spriteheight / 2)
		client_y_ = screenheight - spriteheight / 2;


	x_ = ratio_ * server_x_ + (1 - ratio_) * client_x_;
	y_ = ratio_ * server_y_ + (1 - ratio_) * client_y_;

	if (ratio_ < 1)
	{
		ratio_ += timedelta * 4;
		if (ratio_ > 1)
			ratio_ = 1;
	}

	if (x_ < spritewidth / 2)
		x_ = spritewidth / 2 + 5;
	else if (x_ > screenwidth - spritewidth / 2)
		x_ = screenwidth - spritewidth / 2;

	if (y_ < spriteheight / 2)
		y_ = spriteheight / 2;
	else if (y_ > screenheight - spriteheight / 2)
		y_ = screenheight - spriteheight / 2;
}


/**
* Render Cycle
*
* Renders the ship to the screen. Must be called between a
* Gfx_BeginScene an Gfx_EndScene, otherwise bad things will
* happen.
*/

void Ship::Render()
{
	sprite_->RenderEx(x_, y_, w_);

	font_->printf(x_ + 5, y_ + 5, HGETEXT_LEFT, "%s", mytext_.c_str());
}

/**
* Accelerates a ship by the given acceleration (i.e. increases
* the ships velocity in the direction it is pointing in)
*
* @param acceleration How much to accelerate by in px/s^2
* @param timedelta Time passed since last frame
*/

void Ship::AccelerateY(float acceleration, float timedelta, bool direction)
{
	if (direction)
		server_vely_ -= acceleration * timedelta;
	else
		server_vely_ += acceleration *  timedelta;
}
void Ship::AccelerateX(float acceleration, float timedelta, bool direction)
{
	if (direction)
		server_velx_ += acceleration * timedelta;
	else
		server_velx_ -= acceleration * timedelta;
}
void Ship::SetName(const char * text)
{
	mytext_.clear();
	mytext_ = text;
}

hgeRect* Ship::GetBoundingBox()
{
	sprite_->GetBoundingBox(x_, y_, &collidebox);

	return &collidebox;
}

bool Ship::HasCollided(Ship &ship)
{
	sprite_->GetBoundingBox(x_, y_, &collidebox);

	return collidebox.Intersect(ship.GetBoundingBox());
}