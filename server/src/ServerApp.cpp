#include <iostream>

#include "RakNetworkFactory.h"
#include "RakPeerInterface.h"
#include "Bitstream.h"
#include "GetTime.h"

#include "config.h"
#include "MyMsgIDs.h"

#include "ServerApp.h"
static float tick = 0, last_tick = 0;
float dt = 0;
ServerApp::ServerApp() : 
	rakpeer_(RakNetworkFactory::GetRakPeerInterface()),
	newID(0)
{
    rakpeer_->Startup( 100, 30, &SocketDescriptor( DFL_PORTNUMBER, 0 ), 1 );
    rakpeer_->SetMaximumIncomingConnections( DFL_MAX_CONNECTION );
	rakpeer_->SetOccasionalPing(true);
	std::cout << "Server Started" << std::endl;
	Shiptimer = 0;
	ShipUpdateTimer = 0;
}

ServerApp::~ServerApp()
{
	rakpeer_->Shutdown(100);
	RakNetworkFactory::DestroyRakPeerInterface(rakpeer_);
}

void ServerApp::Loop()
{
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

		switch (msgid)
		{
		case ID_NEW_INCOMING_CONNECTION:
				SendWelcomePackage(packet->systemAddress);
			break;

		case ID_DISCONNECTION_NOTIFICATION:
		case ID_CONNECTION_LOST:
			SendDisconnectionNotification(packet->systemAddress);
			break;

		case ID_INITIALPOS:
			{
				float x_, y_;
                int type_;
				std::cout << "ProcessInitialPosition" << std::endl;
				bs.Read( x_ );
				bs.Read( y_ );
                bs.Read( type_ );
				ProcessInitialPosition( packet->systemAddress, x_, y_, type_);
			}
			break;

		case ID_MOVEMENT:
			// hint : movement stuffs goes here!
				bs.ResetReadPointer();
				rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
			break;
		case ID_NEWMISSILE:
				bs.ResetReadPointer();
				rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
			break;
		case ID_UPDATEMISSILE:
				bs.ResetReadPointer();
				rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
			break;
		case ID_MISSILECOLLIDE:
		{
			int id;
			int missileID;
			int ShipID;
			bs.Read(id);
			bs.Read(missileID);
			bs.Read(ShipID);
			bs.ResetReadPointer();
			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);//let the other clients have the data first
			RemoveShips(ShipID);
			break;
		}
		case ID_TIMEBOMBCREATE:
		{
			float x, y,radius,timer;
			int ShipID;
			bs.Read(x);
			bs.Read(y);
			bs.Read(radius);
			bs.Read(timer);
			bs.Read(ShipID);
			TimeBomb * time_bomb = new TimeBomb(x, y, radius, timer,ShipID);
			timeBomb.push_back(time_bomb);
			bs.ResetReadPointer();
			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
			std::cout << "Get time bomb" << std::endl;
			break;
		}
		case ID_NEWGATEPOSITION:
			bs.ResetReadPointer();
			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
		case ID_POWERUPEND:
			bs.ResetReadPointer();
			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
		default:
			std::cout << "Unhandled Message Identifier: " << (int)msgid << std::endl;
		}
		//server controlled AI
		tick = GetTickCount();
		dt = tick - last_tick;
		dt /= 1000.0f;
		last_tick = tick;
		if (dt < 1)
		{
			Shiptimer += dt;
			ShipUpdateTimer += dt;
		}
		for (int i = 0; i<ships.size(); ++i)
		{
			if (ships[i])
				ships[i]->EnemyUpdate(dt);
		}
		CreateShips();
		SendUpdatedShips(dt, packet->systemAddress);
		UpdateTimeBomb(dt);
		rakpeer_->DeallocatePacket(packet);
	}
}
void ServerApp::RemoveShips(int ShipID,int damage)
{
	for (int i = 0; i < ships.size(); ++i)
	{
		if (ships[i]->GetID() == ShipID)
		{
			ships[i]->health -= damage;
			if (ships[i]->health <= 0)
			{
				ServerScore += 50;
				std::swap(ships[i], ships[ships.size() - 1]);
				Ship*temp = ships[ships.size() - 1];
				ships.pop_back();
				if (rand() % 2)
					CreatePowerUps(temp->GetX(), temp->GetY());
				delete temp;
			}
			break;
		}
	}
}
void ServerApp::CreateShips()
{
	if (Shiptimer > 10.0f)
	{
		RakNet::BitStream bs2;
		float x = (rand() % 50) + 200;
		int ships_amt = rand() % 2 + 1;
		unsigned char msgid = ID_NEWENEMYSHIP;
		unsigned int type = 1;
		//RakNet::BitStream bs;
		Ship* tempship = new Ship(type, x, 200 - 25,5);
		int ID = totalShips;
		tempship->SetVelocityY(3.0f);
		tempship->SetVelocityX(30.0f);
		tempship->setID(ID);
		ships.push_back(tempship);
		bs2.Write(msgid);
		bs2.Write(ID);
		bs2.Write(type);
		bs2.Write(tempship->GetX());
		bs2.Write(tempship->GetY());
		bs2.Write(tempship->health);
		bs2.Write(tempship->GetVelocityX());
		bs2.Write(tempship->GetVelocityY());
		rakpeer_->Send(&bs2, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
		bs2.Reset();
		++totalShips;
		Shiptimer = 0;
	}
}

void ServerApp::SendUpdatedShips(float dt, SystemAddress& addr)
{
	if (ShipUpdateTimer > 1.3f)
	{
		SendScore();
		RakNet::BitStream bs;
		unsigned char msgid = ID_UPDATEENEMYSHIP;
		unsigned int shipid;
		for (int i = 0; i < ships.size(); ++i)
		{
			//bs.ResetReadPointer()		
			shipid = ships[i]->GetID();
			bs.Write(msgid);
			bs.Write(shipid);
			bs.Write(ships[i]->GetX());
			bs.Write(ships[i]->GetY());
			bs.Write(ships[i]->GetVelocityX());
			bs.Write(ships[i]->GetVelocityY());
			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			bs.Reset();
			if (rand() % 3 == 0)
			{
				msgid = ID_NEWMISSILE;
				bs.Write(msgid);
				bs.Write(shipid);
				bs.Write(ships[i]->totalMissileShot);
				bs.Write(ships[i]->GetX());
				bs.Write(ships[i]->GetY());
				bs.Write(-ships[i]->GetW());
				++ships[i]->totalMissileShot;
				rakpeer_->Send(&bs, HIGH_PRIORITY, PacketReliability::RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
				bs.Reset();
			}
		}
		ShipUpdateTimer = 0;
	}
}
void ServerApp::UpdateShips(float dt)
{
	for (int i = 0; i < ships.size(); ++i)
	{
		//ships[i]->AccelerateY(-1, dt, false);
		ships[i]->EnemyUpdate(dt);
	}
}

void ServerApp::CreatePowerUps(float x,float y)
{
	//PowerUp* powerup = new PowerUp(x, y, 0, 10,totalPowerUps);
	//powerUps.push_back(powerup);
	++totalPowerUps;//increase total powerups count;
	RakNet::BitStream bs;
	unsigned char msgid = ID_NEWPOWERUP;
	bs.Write(msgid);
	bs.Write(totalPowerUps);
	bs.Write(x);
	bs.Write(y);
	bs.Write(0.0f);//vel_x
	bs.Write(10.f);//vel_y
	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	bs.Reset();
	std::cout << "PowerUps Created" << std::endl;
}
void ServerApp::RemovePowerUps(int PowerUPID)
{
	for (int i = 0; i < powerUps.size(); ++i)
	{
		if (powerUps[i]->ID == PowerUPID)
		{
			std::swap(powerUps[i], powerUps[powerUps.size() - 1]);
			PowerUp*temp = powerUps[powerUps.size() - 1];
			powerUps.pop_back();
			delete temp;
			break;
		}
	}
}
void ServerApp::UpdateTimeBomb(float dt)
{
	for (int i = 0; i < timeBomb.size(); ++i)
	{
		if (timeBomb[i]->Update(dt))
		{
			RakNet::BitStream bs;
			std::vector<int>shipIdList;
			unsigned char msgid = ID_TIMEBOMBEND;
			bs.Write(msgid);//msgid
			bs.Write(timeBomb[i]->ID);//bomb id
			//explosion codes
			for (int a = 0; a < ships.size(); ++a)
			{
				if ((ships[a]->GetX() - timeBomb[i]->x)*(ships[a]->GetX() - timeBomb[i]->x) +
					(ships[a]->GetY() - timeBomb[i]->y)*(ships[a]->GetY() - timeBomb[i]->y) < 
					timeBomb[i]->Radius * timeBomb[i]->Radius)
				{
					shipIdList.push_back(ships[a]->GetID());
				}
			}
			bs.Write(shipIdList.size());//total ships hit
			for (int a = 0; a < shipIdList.size(); ++a)
			{
				bs.Write(shipIdList[a]);//ship's id
				RemoveShips(shipIdList[a],100);
			}
			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
			bs.Reset();
			std::cout << "time bomb explode killed: " << shipIdList.size() << std::endl;
			std::swap(timeBomb[i], timeBomb[timeBomb.size() - 1]);
			delete timeBomb[timeBomb.size() -1];
			timeBomb.pop_back();
			break;
		}
	}
}

void ServerApp::SendWelcomePackage(SystemAddress& addr)
{
	++newID;
	unsigned int shipcount = static_cast<unsigned int>(clients_.size());
	unsigned char msgid = ID_WELCOME;
	
	RakNet::BitStream bs;
	bs.Write(msgid);
	bs.Write(newID);
	bs.Write(shipcount);

	for (ClientMap::iterator itr = clients_.begin(); itr != clients_.end(); ++itr)
	{
		std::cout << "Ship " << itr->second.id << " pos" << itr->second.x_ << " " << itr->second.y_ << std::endl;
		bs.Write( itr->second.id );
		bs.Write( itr->second.x_ );
		bs.Write( itr->second.y_ );
        bs.Write( itr->second.type_ );
	}

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED,0, addr, false);

	bs.Reset();
	//send in the enemy ships
	msgid = ID_NEWENEMYSHIP;
	for (int i = 0; i < ships.size(); ++i)
	{
		bs.Write(msgid);
		bs.Write(ships[i]->GetID());
		bs.Write(ships[i]->GetType());
		bs.Write(ships[i]->GetX());
		bs.Write(ships[i]->GetY());
		bs.Write(ships[i]->health);
		bs.Write(ships[i]->GetVelocityX());
		bs.Write(ships[i]->GetVelocityY());
		rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, addr, false);
		bs.Reset();
	}
	GameObject newobject(newID);

	clients_.insert(std::make_pair(addr, newobject));

	std::cout << "New guy, assigned id " << newID << std::endl;
}

void ServerApp::SendDisconnectionNotification(SystemAddress& addr)
{
	ClientMap::iterator itr = clients_.find(addr);
	if (itr == clients_.end())
		return;

	unsigned char msgid = ID_LOSTSHIP;
	RakNet::BitStream bs;
	bs.Write(msgid);
	bs.Write(itr->second.id);

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, true);

	std::cout << itr->second.id << " has left the building" << std::endl;

	clients_.erase(itr);

}

void ServerApp::ProcessInitialPosition( SystemAddress& addr, float x_, float y_, int type_ ){
	unsigned char msgid;
	RakNet::BitStream bs;
	ClientMap::iterator itr = clients_.find(addr);
	if (itr == clients_.end())
		return;

	itr->second.x_ = x_;
	itr->second.y_ = y_;
    itr->second.type_ = type_;

	std::cout << "Received pos " << itr->second.x_ << " " << itr->second.y_ << std::endl;
    std::cout << "Received type " << itr->second.type_ << std::endl;

	msgid = ID_NEWSHIP;
    bs.Write( msgid );
    bs.Write( itr->second.id );
    bs.Write( itr->second.x_ );
    bs.Write( itr->second.y_ );
    bs.Write( itr->second.type_ );

	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, true);
}

void ServerApp::UpdatePosition( SystemAddress& addr, float x_, float y_ )
{
    ClientMap::iterator itr = clients_.find( addr );
    if( itr == clients_.end( ) )
        return;

    itr->second.x_ = x_;
    itr->second.y_ = y_;
}
void ServerApp::SendScore()
{
	unsigned char msgid = ID_SCOREUPDATE;
	RakNet::BitStream bs;
	bs.Write(msgid);
	bs.Write(ServerScore);
	rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	bs.Reset();
}