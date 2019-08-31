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

#define OPE_MODE_MANUAL	0x0000
#define OPE_MODE_AUTO	0x0100

#define OPE_MODE_AS_ON	0x0001
#define OPE_MODE_AS_OFF	0x0000


#define AUTO_MODE_STANDBY	0x0000
#define AUTO_MODE_SUSPEND	0x0001
#define AUTO_MODE_INTERRUPT	0x0002
#define AUTO_MODE_ACTIVE	0x0003



class CMODE_Table
{
public:
	CMODE_Table() {};
	~CMODE_Table() {};
	
	DWORD operation;//AUTO MANUAL 
	DWORD antisway; //Antisway ON / OFF
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
	Vector3 lv;		//Ý_vx vy vz


//Ý×Ý_ÔÎĘu
	double L;		//[vˇ
	double lph;		//Z˛ĆĚpx
	double lth;		//XY˝Ępx

	double vL;		//ŞŹx
	double vlph;	//Z˛ĆĚpŹx
	double vlth;	//XY˝ĘpŹx

	double T;		//Uęüú
	double w0;		//Uępüg(2PI()/Tj

	Vector3 PhPlane_r;	//Z˛pxĚĘ˝Ę x:OmegaTheata y:TheataDot
	Vector3 PhPlane_n;	//xy˝ĘźaűüĚĘ˝Ę x:OmegaTheata y:TheataDot
	Vector3 PhPlane_t;	//xy˝ĘÚüűüĚĘ˝Ę x:OmegaTheata y:TheataDot


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
