#include <iostream>

#include "hge.h"

#include "RakNetworkFactory.h"
#include "RakPeerInterface.h"
#include "Bitstream.h"
#include "GetTime.h"

#include "config.h"
#include "MyMsgIDs.h"

#include "Globals.h"
#include "ship.h"
#include "Application.h"
//#include <iostream>
#define BUFFERSIZE 256
/** 
* Constuctor
*
* Creates an instance of the graphics engine and network engine
*/
static int updatemissile = 0;
Application::Application() : 
	hge_(hgeCreate(HGE_VERSION)),
	fpsbox(0),
	rakpeer_(RakNetworkFactory::GetRakPeerInterface()),
	timer_( 0 ),
	totalsent_(0),
	totalreceived_(0),
	have_missile( false ),
	keydown_enter( false ),
    asteroid( 0 )
{
}

/**
* Destructor
*
* Does nothing in particular apart from calling Shutdown
*/

Application::~Application() throw()
{
	delete fpsbox;
	fpsbox = 0;
	delete databox;
	databox = 0;

	Shutdown();
	rakpeer_->Shutdown(100);
	RakNetworkFactory::DestroyRakPeerInterface(rakpeer_);
}

/**
* Initialises the graphics system
* It should also initialise the network system
*/

bool Application::Init()
{
	srand( RakNet::GetTime() );

	hge_->System_SetState(HGE_FRAMEFUNC, Application::Loop);
	hge_->System_SetState(HGE_WINDOWED, true);
	hge_->System_SetState(HGE_USESOUND, false);
	hge_->System_SetState(HGE_TITLE, "DM2241 Multiplayer Game Programming");
//	hge_->System_SetState(HGE_LOGFILE, "movement.log");
	hge_->System_SetState(HGE_DONTSUSPEND, true);

	if(hge_->System_Initiate()) 
	{
        asteroid = new Asteroid( "asteroid.png" );
        
        fpsbox = new TextBox( "font1.fnt" );
		fpsbox->SetPos( 5, 5 );

		databox = new TextBox("font1.fnt");
		databox->SetPos( 580, 5 );

        int ShipType = rand() % 4 + 1;
        float init_pos_x = (float)(rand() % 500 + 100);
        float init_pos_y = (float)(rand() % 400 + 100);
        ships_.push_back( new Ship( ShipType, init_pos_x, init_pos_y ) );
        std::cout << "My Ship: type[" << ShipType << "] x[" << init_pos_x << "] y[" << init_pos_y << "]" << std::endl;
		ships_.at(0)->SetName("My Ship");
		if (rakpeer_->Startup(1,30,&SocketDescriptor(), 1))
		{
			rakpeer_->SetOccasionalPing(true);
            return rakpeer_->Connect( DFL_SERVER_IP, DFL_PORTNUMBER, 0, 0 );
		}
	}
	return false;
}

/**
* Update cycle
*
* Checks for keypresses:
*   - Esc - Quits the game
*   - Left - Rotates ship left
*   - Right - Rotates ship right
*   - Up - Accelerates the ship
*   - Down - Deccelerates the ship
*
* Also calls Update() on all the ships in the universe
*/
bool Application::Update()
{
    if( hge_->Input_GetKeyState( HGEK_ESCAPE ) )
        return true;

    float timedelta = hge_->Timer_GetDelta( );

    ships_.at( 0 )->SetAngularVelocity( 0.0f );

    if( hge_->Input_GetKeyState( HGEK_LEFT ) )
    {
       // ships_.at( 0 )->SetAngularVelocity( ships_.at( 0 )->GetAngularVelocity( ) - DEFAULT_ANGULAR_VELOCITY );
		ships_.at(0)->AccelerateX(DEFAULT_ACCELERATION, timedelta, false);
    }

    if( hge_->Input_GetKeyState( HGEK_RIGHT ) )
    {
		ships_.at(0)->AccelerateX(DEFAULT_ACCELERATION, timedelta, true);
    }

    if( hge_->Input_GetKeyState( HGEK_UP ) )
    {
        ships_.at( 0 )->AccelerateY( DEFAULT_ACCELERATION, timedelta ,true);
    }

    if( hge_->Input_GetKeyState( HGEK_DOWN ) )
    {
        ships_.at( 0 )->AccelerateY( DEFAULT_ACCELERATION, timedelta ,false);
    }

    if( hge_->Input_GetKeyState( HGEK_SPACE ) )
	{
		if( !keydown_enter )
		{
			CreateMissile(ships_.at(0)->GetX(), ships_.at(0)->GetY(), ships_.at(0)->GetW(), ships_.at(0)->GetID(),mymissile.size());  
			keydown_enter = true;
		}
	}
	else
	{
		if( keydown_enter )
		{
			keydown_enter = false;
		}
	}

    // update ships
	for (ShipList::iterator ship = ships_.begin();
		ship != ships_.end(); ship++)
	{
		(*ship)->Update(timedelta);
	}
	for (int i = 0; i < EnemyShips.size(); ++i)
	{
		EnemyShips[i]->EnemyUpdate(timedelta);
	}
    // update asteroid
    if( asteroid )
    {
        if( asteroid->Update( ships_, timedelta ) )
        {
            //	delete asteroid;
            //	asteroid = 0;
        }
    }

	RakNet::BitStream bs;
	unsigned char msgid = 0;
	//update missiles
	for (int i = 0; i < mymissile.size();++i)
	{
		int a = mymissile.at(i)->Update(ships_, timedelta);
		if(a == 1)
		{
			// have collision
			std::swap(mymissile[mymissile.size() - 1], mymissile[i]);
			Missile* temp = mymissile[mymissile.size() - 1];
			mymissile.pop_back();
			msgid = ID_MISSILECOLLIDE;
			bs.Write(msgid);
			bs.Write(temp->GetOwnerID());
			bs.Write(temp->missileID);
			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			bs.Reset();
			delete temp;
		}
		else if (a == 2)
		{
			std::swap(mymissile[mymissile.size() - 1], mymissile[i]);
			Missile* temp = mymissile[mymissile.size() - 1];
			mymissile.pop_back();
			delete temp;
		}
	}
	for (MissileList::iterator missile = missiles_.begin();missile != missiles_.end(); missile++)
	{
		
		if ((*missile)->Update(ships_, timedelta) == 2)
		{
			// have collision
			delete *missile;
			missiles_.erase(missile);
			break;
		}
	}
	if (Packet* packet = rakpeer_->Receive())
	{
		RakNet::BitStream bs(packet->data, packet->length, false);
		
		unsigned char msgid = 0;
		RakNetTime timestamp = 0;

		bs.Read(msgid);

		if (msgid == ID_TIMESTAMP)
		{
			bs.Read(timestamp);
			bs.Read(msgid);
		}

		switch(msgid)
		{
		case ID_CONNECTION_REQUEST_ACCEPTED:
			std::cout << "Connected to Server" << std::endl;
			break;

		case ID_NO_FREE_INCOMING_CONNECTIONS:
		case ID_CONNECTION_LOST:
		case ID_DISCONNECTION_NOTIFICATION:
			std::cout << "Lost Connection to Server" << std::endl;
			rakpeer_->DeallocatePacket(packet);
			return true;

		case ID_WELCOME:
			{
                unsigned int shipcount, id;
                float x_, y_;
                int type_;
                std::string temp;
                char chartemp[5];

                bs.Read( id );
                ships_.at( 0 )->setID( id );
                bs.Read( shipcount );

                for( unsigned int i = 0; i < shipcount; ++i )
                {
                    bs.Read( id );
                    bs.Read( x_ );
                    bs.Read( y_ );
                    bs.Read( type_ );
                    std::cout << "Welcome Ship pos " << x_ << ", " << y_ << ", type " << type_ << std::endl;
                    Ship* ship = new Ship( type_, x_, y_ );
                    temp = "Ship ";
                    temp += _itoa_s( id, chartemp, 10 );
                    ship->SetName( temp.c_str( ) );
                    ship->setID( id );
                    ships_.push_back( ship );
                }

                SendInitialPosition( );
			}
			break;

		case ID_NEWSHIP:
			{
				unsigned int id;
				bs.Read(id);

				if( id == ships_.at(0)->GetID() )
				{
					// if it is me
					break;
				}
				else
				{
					float x_, y_;
					int type_;
					std::string temp;
					char chartemp[5];

					bs.Read( x_ );
					bs.Read( y_ );
					bs.Read( type_ );
					std::cout << "New Ship pos" << x_ << " " << y_ << std::endl;
					Ship* ship = new Ship(type_, x_, y_);
					temp = "Ship "; 
					temp += _itoa_s(id, chartemp, 10);
					ship->SetName(temp.c_str());
					ship->setID( id );
					ships_.push_back(ship);
				}
			}
			break;

		case ID_LOSTSHIP:
			{
				unsigned int shipid;
				bs.Read(shipid);
				for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr)
				{
                    if( (*itr)->GetID( ) == shipid )
					{
						delete *itr;
						ships_.erase(itr);
						break;
					}
				}
			}
			break;

		case ID_INITIALPOS:
			break;

		case ID_MOVEMENT:
			{
				totalreceived_ += bs.GetNumberOfBytesUsed();
				
				unsigned int shipid;
                float server_x, server_y, server_w;
                float server_vel_x, server_vel_y, server_vel_angular;
				bs.Read(shipid);
				for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); ++itr)
				{
					if ((*itr)->GetID() == shipid)
					{
                        bs.Read( server_x );
                        bs.Read( server_y );
                        bs.Read( server_w );
                        bs.Read( server_vel_x );
                        bs.Read( server_vel_y );
                        bs.Read( server_vel_angular );

                        (*itr)->SetServerLocation( server_x, server_y, server_w );
                        (*itr)->SetServerVelocity( server_vel_x, server_vel_y, server_vel_angular );
                        (*itr)->DoInterpolateUpdate();
						break;
					}
				}
			}
			break;

        case ID_COLLIDE:
            {
                unsigned int shipid;
                float x, y;
                bs.Read( shipid );

                if( shipid == ships_.at( 0 )->GetID( ) )
                {
                    std::cout << "collided with someone!" << std::endl;
                    bs.Read( x );
                    bs.Read( y );
                    ships_.at( 0 )->SetX( x );
                    ships_.at( 0 )->SetY( y );
                    bs.Read( x );
                    bs.Read( y );
                    ships_.at( 0 )->SetVelocityX( x );
                    ships_.at( 0 )->SetVelocityY( y );
    #ifdef INTERPOLATEMOVEMENT
                    bs.Read( x );
                    bs.Read( y );
                    ships_.at( 0 )->SetServerVelocityX( x );
                    ships_.at( 0 )->SetServerVelocityY( y );
    #endif	
                }
            }
            break;


		case ID_NEWMISSILE:
			{
				float x, y, w;
				int id;
				int missileID;
				bs.Read(id);
				bs.Read(missileID);
				bs.Read(x);
				bs.Read(y);
				bs.Read(w);

				missiles_.push_back( new Missile( "missile.png", x, y, w, id ,missileID) );
			}
			break;
		case ID_UPDATEMISSILE:
			{
				float x,y,w;
				int id;
				int missileID;

				bs.Read(id);
				bs.Read(missileID);

				for (MissileList::iterator itr = missiles_.begin(); itr != missiles_.end(); ++itr)
				{
					if ((*itr)->missileID == missileID )
					{
						if ((*itr)->GetOwnerID() == id)
						{
							bs.Read( x );
							bs.Read( y );
							bs.Read( w );
							(*itr)->UpdateLoc( x, y, w );
							break;
						}
					}
				}
				
			}
			break;
		case ID_MISSILECOLLIDE:
		{
			int id;
			int missileID;

			bs.Read(id);
			bs.Read(missileID);

			for (MissileList::iterator itr = missiles_.begin(); itr != missiles_.end(); ++itr)
			{
				 if ((*itr)->missileID == missileID && (*itr)->GetOwnerID() == id)
				 {
					 delete (*itr);
					 missiles_.erase(itr);
					 break;
				 }
			}
			break;
		}
		case ID_NEWENEMYSHIP:
		{
			int type;
			unsigned char ID;
			float x, y;
			bs.Read(ID);
			bs.Read(type);
			bs.Read(x);
			bs.Read(y);
			Ship* ship = new Ship(type, x, y);
			ship->setID(ID);
			EnemyShips.push_back(ship);
			break;
		}
		case ID_UPDATEENEMYSHIP:
		{
			unsigned int shipid = 0;
			float server_x, server_y;
			float server_vel_x, server_vel_y;
			
			bs.Read(shipid);
			bs.Read(server_x);
			bs.Read(server_y);
			bs.Read(server_vel_x);
			bs.Read(server_vel_y);
			//shipid /= 16777216;
			std::cout << shipid << " SHIP_ID" << server_vel_y << std::endl;
			
			EnemyShips[shipid]->SetX(server_x);
			EnemyShips[shipid]->SetY(server_y);
	
			EnemyShips[shipid]->SetVelocityX(server_vel_x);
			EnemyShips[shipid]->SetVelocityY(server_vel_y);
			//EnemyShips[shipid]->DoInterpolateUpdate();
			break;
		}

			break;
		default:
			std::cout << "Unhandled Message Identifier: " << (int)msgid << std::endl;

		}
		rakpeer_->DeallocatePacket(packet);
	}

	if (RakNet::GetTime() - timer_ > 100)
	{
		timer_ = RakNet::GetTime();

		RakNet::BitStream bs2;
		unsigned char msgid = ID_MOVEMENT;
        bs2.Write( msgid );
        bs2.Write( ships_.at( 0 )->GetID() );
        bs2.Write( ships_.at( 0 )->GetServerX() );
        bs2.Write( ships_.at( 0 )->GetServerY() );
        bs2.Write( ships_.at( 0 )->GetServerW() );
        bs2.Write( ships_.at( 0 )->GetServerVelocityX() );
        bs2.Write( ships_.at( 0 )->GetServerVelocityY() );
        bs2.Write( ships_.at( 0 )->GetAngularVelocity() );

		rakpeer_->Send(&bs2, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
		totalsent_ += bs2.GetNumberOfBytesUsed();

		RakNet::BitStream bs3;
		unsigned char msgid2 = ID_UPDATEMISSILE;
		unsigned char deleted = 0;
	}

	return false;
}


/**
* Render Cycle
*
* Clear the screen and render all the ships
*/
void Application::Render()
{
	std::string fps;
	char temp[BUFFERSIZE];

	hge_->Gfx_BeginScene();
	hge_->Gfx_Clear(0);

    // render astroid
    if( asteroid )
    {
        asteroid->Render( );
    }

    // render spaceships
	ShipList::iterator itr;
	for (itr = ships_.begin(); itr != ships_.end(); itr++)
	{
		(*itr)->Render();
	}
	for (int i = 0; i < EnemyShips.size(); ++i)
	{
		EnemyShips[i]->EnemyRender();
	}
	for (int i = 0; i < mymissile.size(); ++i)
	{
		mymissile.at(i)->Render();
	}
	MissileList::iterator itr2;
	for (itr2 = missiles_.begin(); itr2 != missiles_.end(); itr2++)
	{
		(*itr2)->Render();
	}

	hge_->Gfx_EndScene();
}


/** 
* Main game loop
*
* Processes user input events
* Supposed to process network events
* Renders the ships
*
* This is a static function that is called by the graphics
* engine every frame, hence the need to loop through the
* global namespace to find itself.
*/
bool Application::Loop()
{
	Global::application->Render();
	return Global::application->Update();
}

/**
* Shuts down the graphics and network system
*/

void Application::Shutdown()
{
	ShipList::iterator itr = ships_.begin();

	while( itr != ships_.end() )
	{
		delete *itr;
		ships_.erase(itr);
		itr = ships_.begin();
		if( itr == ships_.end() ) break;
//		itr++;
	//	if( itr == ships_.end()

	}
/*
	for (ShipList::iterator itr = ships_.begin(); itr != ships_.end(); itr++)
	{			
		delete *itr;
		ships_.erase(itr);
		itr = ships_.begin();
	}
*/
	hge_->System_Shutdown();
	hge_->Release();
}

/** 
* Kick starts the everything, called from main.
*/
void Application::Start()
{
	if (Init())
	{
		hge_->System_Start();
	}
}

bool Application::SendInitialPosition()
{
	RakNet::BitStream bs;
	unsigned char msgid = ID_INITIALPOS;
	bs.Write(msgid);
	bs.Write(ships_.at(0)->GetX());
	bs.Write(ships_.at(0)->GetY());
    bs.Write( ships_.at( 0 )->GetType( ) );
	
	std::cout << "Sending pos " << ships_.at(0)->GetX() << ", " << ships_.at(0)->GetY() << ", type " << ships_.at(0)->GetType() << std::endl;

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

	return true;
}

void Application::CreateMissile(float x, float y, float w, int id,int missileID)
{
	RakNet::BitStream bs;
	unsigned char msgid;
	unsigned char deleted=0;

	if( id != ships_.at(0)->GetID() )
	{
		// not my ship
		missiles_.push_back(new Missile("missile.png", x, y, w, id, missileID));
	}
	else
	{		
		//if( have_missile )
		//{
		//	//locate existing missile

		//	// send network command to delete across all clients
		//		deleted=1;
		//		msgid = ID_UPDATEMISSILE;
		//		bs.Write(msgid);
		//		bs.Write(id);
		//		bs.Write(deleted);
		//		bs.Write(x);
		//		bs.Write(y);
		//		bs.Write(w);
		//		
		//		rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

		//		have_missile = false;
		//}
		//
		// add new missile to list
		mymissile.push_back(new Missile("missile.png", x, y, w, id, missileID));
			
		// send network command to add new missile
		bs.Reset();
		msgid = ID_NEWMISSILE;
		bs.Write(msgid);
		bs.Write(id);
		bs.Write(missileID);
		bs.Write(x);
		bs.Write(y);
		bs.Write(w);
		
		rakpeer_->Send(&bs, HIGH_PRIORITY,PacketReliability::RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

		have_missile = true;
	}
}