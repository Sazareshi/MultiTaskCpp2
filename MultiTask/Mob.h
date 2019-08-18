#pragma once

#include "CVector3.h"
#include "CRK4.h"

#define GRAVITY_ACC		9.81//Gravitational acceleration

#define DEFAULT_HP_X	0.0		//Default Position X
#define DEFAULT_HP_Y	0.0		//Default Position Y
#define DEFAULT_HP_Z	25.0	//Default Position Z
#define DEFAULT_ROPE_L	15.0	//Default Rope L
#define DEFAULT_SLEW	0.0		//Default Position SLEW
#define DEFAULT_BH		20.0	//Default Position BH

#define HP_Tf		    0.1		//�w�ߒl�ɑ΂���ꎟ�x�ꎞ�萔


class Mob_HoistPoint:public RK4
{
public:
	Mob_HoistPoint();
	~Mob_HoistPoint();

	void init_mob(double t, Vector3& r, Vector3& v);
	Vector3 A(double t, Vector3& a, Vector3& v) ; //Model of acceleration  t:no use,a:acc ref,v:no use
	void timeEvolution(double t);

	Vector3 acc_cyl;	//�����x �� r z
	Vector3 acc_cyl_ref; //�����x�w��  �� r z
	Vector3 acc_rec;	//�����x �� r z
	
	double l_h;	//���[�v��
	double v_h;	//�����x;
	double a_h;	//�������x;

	double th_sl;	//����p�x
	double w_sl;	//����p���x
	double dw_sl;	//����p�����x

	double r_bm;	//�����ʒu
	double v_bm;	//�������x
	double a_bm;	//���������x

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
	double m;	//����
};

