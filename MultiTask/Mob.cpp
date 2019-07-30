#include "stdafx.h"
#include "Mob.h"

//Mob
Mob::Mob(){}
Mob::~Mob(){}

//Mob_HP
Mob_HoistPoint::Mob_HoistPoint() {};
Mob_HoistPoint::~Mob_HoistPoint() {};

void Mob_HoistPoint::init_mob() {
	Vector3 init_pos(DEFAULT_HP_X, DEFAULT_HP_Y, DEFAULT_HP_Z);
	set_pos(init_pos);
	return;
};

//Mob_HL
Mob_HungLoad::Mob_HungLoad() {};
Mob_HungLoad::~Mob_HungLoad() {};

void Mob_HungLoad::init_mob() {
	Vector3 init_pos(DEFAULT_HL_X, DEFAULT_HL_Y, DEFAULT_HL_Z);
	set_pos(init_pos);
	return;
};
