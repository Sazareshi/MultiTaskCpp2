#pragma once

#include "CommonFormat.h"
#include "CVector3.h"

class CORDER_Table
{
public:
	CORDER_Table();
	~CORDER_Table();

	ST_JOB_ORDER job_A;
	ST_JOB_ORDER job_B;
	ST_MODE_ORDER mode;
	ST_ESTOP_ORDER e_stop;
	ST_MANUAL_ORDER manualA;
	ST_MANUAL_ORDER manualB;
	ST_UI_ORDER ui;
};

#define ENV_MODE_REAL	0
#define ENV_MODE_SIM1	1
#define ENV_MODE_SIM2	2

class CMODE_Table
{
public:
	CMODE_Table() {};
	~CMODE_Table() {};
	
	DWORD operation;
	DWORD auto_control;
	DWORD environment;
};

#define NUM_OF_TASK_FAULT	100
#define NUM_OF_TASK			7

class CFAULT_Table
{
public:
	CFAULT_Table() {};
	~CFAULT_Table() {};

	BYTE fault[NUM_OF_TASK][NUM_OF_TASK_FAULT];
};


typedef struct _stIO_Physic {
	double M_load;//吊荷質量

//吊点　　座標原点　xy：旋回軸　z：起伏視点(上が+）
	Vector3 cp;		//吊点xyz
	double R;		//軸長さ
	double th;		//旋回角度
	double ph;		//起伏角度

	Vector3 cv;		//吊点vx vy vz
	double vR;		//軸長さ変化速度
	double wth;		//旋回角速度
	double wph;		//起伏角速度

//吊荷点　　座標原点　吊点
	Vector3 pos;		//吊点xyz
	double L;		//ロープ長
	double Le;		//有効ロープ長（振れ周期）
	double _th;		//水平面角度
	double _ph;		//Z軸角度

	Vector3 vel;		//吊点vx vy vz
	double vL;		// ロープ長変化速度
	double _wth;	//水平面角速度
	double _wph;	//Z軸角速度
}ST_IO_PHYSIC, *LPST_IO_PHYSIC;

class CIO_Table
{
public:
	CIO_Table() {};
	~CIO_Table() {};

	ST_IO_PHYSIC physics;

};
