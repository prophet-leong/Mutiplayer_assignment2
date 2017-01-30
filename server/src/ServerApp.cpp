#include <iostream>

#include "RakNetworkFactory.h"
#include "RakPeerInterface.h"
#include "Bitstream.h"
#include "GetTime.h"

#include "config.h"
#include "MyMsgIDs.h"

#include "ServerApp.h"

ServerApp::ServerApp() : 
	rakpeer_(RakNetworkFactory::GetRakPeerInterface()),
	newID(0)
{
    rakpeer_->Startup( 100, 30, &SocketDescriptor( DFL_PORTNUMBER, 0 ), 1 );
    rakpeer_->SetMaximumIncomingConnections( DFL_MAX_CONNECTION );
	rakpeer_->SetOccasionalPing(true);
	std::cout << "Server Started" << std::endl;
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
		float dt = RakNet::GetTime();
		Shiptimer += dt;
		ShipUpdateTimer += dt;
		if (Shiptimer > 5.0f)
		{

			RakNet::BitStream bs2;
			float x = (rand() % 50)-25;
			int ships_amt = rand() % 7+1;
			unsigned char msgid = ID_NEWENEMYSHIP;
			unsigned int type = 1;
			//RakNet::BitStream bs;
			for (int i = 0; i < ships_amt; ++i)
			{
				Ship* tempship = new Ship(type, x, 100 + 25* i);
				tempship->setID(ships.size());
				ships.push_back(tempship);
				bs2.Write(msgid);
				bs2.Write((unsigned int)tempship->GetID()+1);
				bs2.Write(type);
				bs2.Write(tempship->GetX());
				bs2.Write(tempship->GetY());
				rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
				bs2.Reset();
			}
			Shiptimer = 0;
		}
		if (ShipUpdateTimer > 0.1f)
		{
			RakNet::BitStream bs3;
			for (int i = 0; i < ships.size(); ++i)
			{
				bs3.Reset();
				ships[i]->AccelerateY(1.f, dt, false);
				ships[i]->EnemyUpdate(dt);
				bs3.Write(ID_UPDATEENEMYSHIP);
				bs3.Write(ships[i]->GetID());
				bs3.Write(ships[i]->GetX());
				bs3.Write(ships[i]->GetY());
				bs3.Write(ships[i]->GetVelocityX());
				bs3.Write(ships[i]->GetVelocityX());
				rakpeer_->Send(&bs, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, true);
			}
			ShipUpdateTimer = 0;
		}
		rakpeer_->DeallocatePacket(packet);
	}
}
void ServerApp::UpdateShips(float dt)
{
	for (int i = 0; i < ships.size(); ++i)
	{
		ships[i]->AccelerateY(0.1, dt, false);
		ships[i]->Update(dt);
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