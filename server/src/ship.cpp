#include <math.h>
#include <iostream>

#include "hge.h"
//#include "hgeSprite.h"
//#include "hgeFont.h"

#include "ship.h"

/**
* Ship Constructor
*
* It will load the file specified into a sprite and intialise its
* hotspot to the center. Assumes a sprite size of 64*64 and a
* screen size of 800*600
*
* @param filename Name of the graphics file used to represent the ship
*/

const float PI = 3.141592f*2;

Ship::Ship( int type, float locx_, float locy_,int health_)
: w_( -PI/4 )
, angular_velocity( 0 )
, velocity_x_( 0 )
, velocity_y_( 0 )
, id( 0 )
, collidetimer( 0 )
#ifdef INTERPOLATEMOVEMENT
, server_w_(-PI / 4)
, client_w_(-PI / 4)
, server_velx_( 0 )
, server_vely_( 0 )
, ratio_( 1 )
, health(health_)
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

    //HGE* hge = hgeCreate( HGE_VERSION );

    //switch( type )
    //{
    //case 2:
    //    tex_ = hge->Texture_Load( SHIPTYPE2 );
    //    type_ = 2;
    //    break;
    //case 3:
    //    tex_ = hge->Texture_Load( SHIPTYPE3 );
    //    type_ = 3;
    //    break;
    //case 4:
    //    tex_ = hge->Texture_Load( SHIPTYPE4 );
    //    type_ = 4;
    //    break;
    //default:
    //    tex_ = hge->Texture_Load( SHIPTYPE1 );
    //    type_ = 1;
    //    break;
    //}

 /*   hge->Release( );
    sprite_.reset( new hgeSprite( tex_, 0, 0, 64, 64 ) );

    font_.reset( new hgeFont( "font1.fnt" ) );
    font_->SetScale( 0.5 );
    sprite_->SetHotSpot( 32, 32 );*/

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
	/*HGE* hge = hgeCreate(HGE_VERSION);
	hge->Release();*/

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
}

void Ship::EnemyUpdate(float timedelta)
{
	x_ += velocity_x_ * timedelta;
	y_ += velocity_y_*timedelta;
	if (x_ > 800 && velocity_x_ > 0 || x_ < 0 && velocity_x_ < 0)
		velocity_x_ *= -1;
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
	//sprite_->RenderEx(x_, y_, w_);

	//font_->printf(x_+5, y_+5, HGETEXT_LEFT, "%s", mytext_.c_str());
}

void Ship::EnemyRender()
{
	//sprite_->RenderEx(x_, y_, w_);
}
/**
* Accelerates a ship by the given acceleration (i.e. increases
* the ships velocity in the direction it is pointing in)
*
* @param acceleration How much to accelerate by in px/s^2
* @param timedelta Time passed since last frame
*/

void Ship::AccelerateY(float acceleration, float timedelta,bool direction)
{
	if (direction)
		server_vely_ -= acceleration * timedelta;
	else
		server_vely_ += acceleration *  timedelta;
}
void Ship::AccelerateX(float acceleration, float timedelta,bool direction)
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
	//sprite_->GetBoundingBox( x_, y_, &collidebox );

	return 0;//&collidebox;
}

bool Ship::HasCollided( Ship &ship )
{
    //sprite_->GetBoundingBox( x_, y_, &collidebox );

	return 0;// collidebox.Intersect(ship.GetBoundingBox());
}