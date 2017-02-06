#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <vector>

#include "asteroid.h"
#include "ship.h"
#include "missile.h"
#include "textbox.h"
#include "PowerUp.h"
#include "Collided_Position.h"
#include "TimeBomb.h"
#include "RespawnGate.h"
#include "PrintTextOnScreen.h"
class HGE;
class RakPeerInterface;
//! The default angular velocity of the ship when it is in motion
static const float DEFAULT_ANGULAR_VELOCITY = 3.0f; 
//! The default acceleration of the ship when powered
static const float DEFAULT_ACCELERATION = 50.0f;

/**
* The application class is the main body of the program. It will
* create an instance of the graphics engine and execute the
* Update/Render cycle.
*
*/

class Application
{
	HGE* hge_; //!< Instance of the internal graphics engine
	typedef std::vector<Ship*> ShipList;  //!< A list of ships
	typedef std::vector<Missile*> MissileList;
	ShipList ships_; //!< List of all the ships in the universe
	ShipList EnemyShips;
	std::vector<PowerUp*>powerUps;
	std::vector<Collided_Position*>CollidedPosition;
	std::vector<TimeBomb*>timebomb;
	MissileList missiles_;
	MissileList mymissile;
    Asteroid *asteroid;
	TextBox	*fpsbox;
	TextBox *databox;
	RakPeerInterface* rakpeer_;
	unsigned int timer_;

	float gateSetCooldown;
	RespawnGate*respawnPosition;
	
	int score;
	int weaponType;
	int totalsent_;
	int totalreceived_;
	//missile upgrades allows you to shoot more missile
	unsigned int WeaponUpgradeTier;
	bool have_bomb;
	float shootingCooldown;

	bool Init();
	static bool Loop();
	void Shutdown();
	void ProcessWelcomePackage();
	bool SendInitialPosition();
	void CreateMissile( float x, float y, float w, int id ,int missileID);
	bool RemoveMissile( float x, float y, float w, int id ,int missileID);
	void addPointsBy(int i);
public:
	Application();
	~Application() throw();

	void Start();
	bool Update();
	void Render();
	
};

#endif