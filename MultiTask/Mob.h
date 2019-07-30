#pragma once

#include "CVector3.h"
#include "CRK4.h"

class Mob :RK4
{
public:
	Mob();
	~Mob();

	Vector3 a; //加速度ベクトル

	void set_pos(Vector3 _p) { r.copy(_p); return; }
	virtual void init_mob() { return; }
};

#define DEFAULT_HP_X	0.0
#define DEFAULT_HP_Y	0.0
#define DEFAULT_HP_Z	20.0

class Mob_HoistPoint:Mob
{
public:
	Mob_HoistPoint() ;
	~Mob_HoistPoint();

	void init_mob();
};

#define DEFAULT_HL_X	0.0
#define DEFAULT_HL_Y	0.0
#define DEFAULT_HL_Z	5.0
class Mob_HungLoad :Mob
{
public:
	Mob_HungLoad();
	~Mob_HungLoad();

	void init_mob();
};

