#ifndef SERVERAPP_H_
#define SERVERAPP_H_

#include <map>
#include "RakNetTypes.h"
#include <vector>
#include "ship.h"
#include "PowerUp.h"
#include "TimeBomb.h"
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
	unsigned totalShips = 0;
	unsigned totalPowerUps = 0;
    unsigned int newID;
	float Shiptimer = 0;
	float ShipUpdateTimer = 0;
	int ServerScore = 0;
	std::vector<Ship*>ships;
	std::vector<PowerUp*>powerUps;
	std::vector<TimeBomb*>timeBomb;
	void SendWelcomePackage(SystemAddress& addr);
	void SendDisconnectionNotification(SystemAddress& addr);
	void ProcessInitialPosition( SystemAddress& addr, float x_, float y_, int type_);
    void UpdatePosition( SystemAddress& addr, float x_, float y_ );
	void UpdateShips(float dt);
	void UpdateTimeBomb(float dt);
	//creations for gameobj
	void CreatePowerUps(float x,float y);
	void RemovePowerUps(int PowerUPID);
	void CreateShips();
	void RemoveShips(int ShipId,int damage = 1);
	void SendUpdatedShips(float dt, SystemAddress& addr);
	void SendScore();
public:
	ServerApp();
	~ServerApp();
	void Loop();
};

#endif