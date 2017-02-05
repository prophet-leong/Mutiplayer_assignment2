#pragma once
#ifndef POWERUP_H
#define POWERUP_H

class PowerUp
{
public:
	PowerUp(float x,float y,float vel_x,float vel_y,int ID);
	~PowerUp();
	void Update(float dt);
	int ID;
private:
	float x;
	float y;
	float vel_x;
	float vel_y;
};

#endif // !POWERUP_H

