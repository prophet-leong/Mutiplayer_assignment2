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
				bs.ResetReadPointer();
				rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
			break;
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
			ships[i]->EnemyUpdate(dt);
		}
		if (Shiptimer > 10.0f)
		{
			RakNet::BitStream bs2;
			float x = (rand() % 50)+200;
			int ships_amt = rand() % 2+1;
			unsigned char msgid = ID_NEWENEMYSHIP;
			unsigned int type = 1;
			//RakNet::BitStream bs;
			for (int i = 0; i < ships_amt; ++i)
			{
				Ship* tempship = new Ship(type, x, 200 - 25* i);
				int ID = ships.size();
				tempship->setID(ID);
				ships.push_back(tempship);
				bs2.Write(msgid);
				bs2.Write(ID);
				bs2.Write(type);
				bs2.Write(tempship->GetX());
				bs2.Write(tempship->GetY());
				std::cout << tempship->GetX() << "    " << tempship->GetY() << std::endl;
				rakpeer_->Send(&bs2, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, false);
				bs2.Reset();
			}
			Shiptimer = 0;
		}
		SendUpdatedShips(dt, packet->systemAddress);
		rakpeer_->DeallocatePacket(packet);
	}
}
void ServerApp::SendUpdatedShips(float dt, SystemAddress& addr)
{
	if (ShipUpdateTimer > 1.3f)
	{
		RakNet::BitStream bs;
		unsigned char msgid = ID_UPDATEENEMYSHIP;
		for (int i = 0; i < ships.size(); ++i)
		{
			//bs.ResetReadPointer();
			ships[i]->SetVelocityY(3.0f);
			ships[i]->SetVelocityX(10.0f);
			unsigned int shipid = ships[i]->GetID();
			bs.Write(msgid);
			bs.Write(shipid);
			bs.Write(ships[i]->GetX());
			bs.Write(ships[i]->GetY());
			bs.Write(ships[i]->GetVelocityX());
			bs.Write(ships[i]->GetVelocityY());
			//std::cout << ships[i]->GetX() << "  velocity Y  " << ships[i]->GetY() << std::endl;
			rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, addr, false);
			bs.Reset();
		}
		ShipUpdateTimer = 0;
	}
}
void ServerApp::UpdateShips(float dt)
{
	for (int i = 0; i < ships.size(); ++i)
	{
		ships[i]->AccelerateY(-1, dt, false);
		ships[i]->EnemyUpdate(dt);
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