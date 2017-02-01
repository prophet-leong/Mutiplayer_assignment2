#ifndef SERVERAPP_H_
#define SERVERAPP_H_

#include <map>
#include "RakNetTypes.h"
#include <vector>
#include "ship.h"
class RakPeerInterface;

struct GameObject 
{
	GameObject(unsigned int newid)
	: x_(0), y_(0)
	{
		id = newid;
	}

	unsigned int id;
	float x_;
	float y_;
    int type_;
};

class ServerApp
{
	RakPeerInterface* rakpeer_;
	typedef std::map<SystemAddress, GameObject> ClientMap;

	ClientMap clients_;
	HGE* hge_;
    unsigned int newID;
	float Shiptimer = 0;
	float ShipUpdateTimer = 0;
	std::vector<Ship*>ships;
	void SendWelcomePackage(SystemAddress& addr);
	void SendDisconnectionNotification(SystemAddress& addr);
	void ProcessInitialPosition( SystemAddress& addr, float x_, float y_, int type_);
    void UpdatePosition( SystemAddress& addr, float x_, float y_ );
	void UpdateShips(float dt);
	void SendUpdatedShips(float dt, SystemAddress& addr);
public:
	ServerApp();
	~ServerApp();
	void Loop();
};

#endif