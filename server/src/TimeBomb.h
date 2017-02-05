#pragma once
#include <vector>

class TimeBomb
{
public:
	TimeBomb(float x,float y,float Radius,float time,int ID);
	~TimeBomb();
	bool Update(float dt);
	float Radius;
	int ID;
	float x;
	float y;
	float time;
private:
};

