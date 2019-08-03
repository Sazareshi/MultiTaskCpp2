#pragma once

#include "CVector3.h"
#include "CRK4.h"

#define GRAVITY_ACC		9.81//Gravitational acceleration

#define DEFAULT_HP_X	0.0	//Default Position X
#define DEFAULT_HP_Y	0.0	//Default Position Y
#define DEFAULT_HP_Z	25.0//Default Position Z
#define DEFAULT_ROPE_L	15.0//Default Position Z


class Mob_HoistPoint:public RK4
{
public:
	Mob_HoistPoint();
	~Mob_HoistPoint();

	void init_mob(double t, Vector3& r, Vector3& v);
	Vector3 A(double t, Vector3& r, Vector3& v) ; //Model of acceleration

	Vector3 a; //加速度
	
	double l_h;	//ロープ長
	double v_h;	//巻速度;
	double a_h;	//巻加速度;

	double th_sl;	//旋回角度
	double w_sl;	//旋回角速度
	double dw_sl;	//旋回角加速度

	double r_bm;	//引込位置
	double v_bm;	//旋回角速度
	double a_bm;	//旋回角加速度

};

#define compensationK 0.5
#define compensationGamma 0.5

class Mob_HungLoad :public RK4
{
public:
	Mob_HungLoad();
	~Mob_HungLoad();

	void init_mob(double t, Vector3& r, Vector3& v);
	Vector3 A(double t, Vector3& r, Vector3& v) ; //Model of acceleration
	double S();	//Rope tension
	Mob_HoistPoint* pHP;
	double m;	//質量
};

