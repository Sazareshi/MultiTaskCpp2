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

#define OPE_MODE_MANUAL	0
#define OPE_MODE_AUTO	1


#define AUTO_MODE_STANDBY	0
#define AUTO_MODE_SUSPEND	1
#define AUTO_MODE_INTERRUPT	2
#define AUTO_MODE_ACTIVE	3



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

//吊点　　座標原点　xy：旋回軸　z：地面(上が+）
	Vector3 cp;		//吊点xyz
	double R;		//軸長さ
	double th;		//旋回角度
	double ph;		//起伏角度

	Vector3 cv;		//吊点vx vy vz
	double vR;		//軸長さ変化速度
	double wth;		//旋回角速度
	double wph;		//起伏角速度

//吊荷　　座標原点　xy：旋回軸　z：地面(上が+）
	Vector3 lp;		//吊点xyz
	Vector3 lv;		//吊点vx vy vz


//吊荷吊点間相対位置
	double L;		//　ロープ長
	double lph;		//　Z軸との角度
	double lth;		//  XY平面角度
	double vL;		//巻速度


	double T;		//振れ周期
	double w0;		//振れ角周波数(2PI()/T）

}ST_IO_PHYSIC, *LPST_IO_PHYSIC;

typedef struct _stIO_Ref {

	double slew_w;		//旋回角速度
	double hoist_v;		//旋回角速度
	double bh_v;		//引込速度

}ST_IO_REF, *LPST_IO_REF;




class CIO_Table
{
public:
	CIO_Table() {};
	~CIO_Table() {};

	ST_IO_PHYSIC	physics;
	ST_IO_REF		ref;

};
