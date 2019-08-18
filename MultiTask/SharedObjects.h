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
	double M_load;//Ý×żĘ

//Ý_@@ŔW´_@xyFůń˛@zFnĘ(ăŞ+j
	Vector3 cp;		//Ý_xyz
	double R;		//˛ˇł
	double th;		//ůńpx
	double ph;		//Npx

	Vector3 cv;		//Ý_vx vy vz
	double vR;		//˛ˇłĎťŹx
	double wth;		//ůńpŹx
	double wph;		//NpŹx

//Ý×@@ŔW´_@xyFůń˛@zFnĘ(ăŞ+j
	Vector3 lp;		//Ý_xyz
	double L;		//[vˇ
	double Le;		//Lř[vˇiUęüúj
	double _th;		//˝Ępx
	double _ph;		//Z˛px

	Vector3 lv;		//Ý_vx vy vz
	double vL;		// [vˇĎťŹx
	double _wth;	//˝ĘpŹx
	double _wph;	//Z˛pŹx

	Vector3 lsway_cyl;	//~ŔW@×Uę
	Vector3 lvsway_cyl; //~ŔW@×UęŹx

	Vector3 lsway_rec;	//źsŔW@×Uę
	Vector3 lvsway_rec; //źsŔW@×UęŹx

}ST_IO_PHYSIC, *LPST_IO_PHYSIC;

typedef struct _stIO_Ref {

	double slew_w;		//ůńpŹx
	double hoist_v;		//ůńpŹx
	double bh_v;		//řŹx

}ST_IO_REF, *LPST_IO_REF;




class CIO_Table
{
public:
	CIO_Table() {};
	~CIO_Table() {};

	ST_IO_PHYSIC	physics;
	ST_IO_REF		ref;

};
