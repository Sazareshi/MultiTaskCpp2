#pragma once

#include "CommonFormat.h"
#include "CVector3.h"

class CCOM_Table
{
public:
	CCOM_Table();
	~CCOM_Table();

	ST_JOB_ORDER job_A;
	ST_JOB_ORDER job_B;
	ST_MODE_ORDER mode;
	ST_ESTOP_ORDER e_stop;
	ST_MANUAL_ORDER manualA;
	ST_MANUAL_ORDER manualB;
	ST_UI_ORDER ui;
};

class CMODE_Table
{
public:
	CMODE_Table() {};
	~CMODE_Table() {};
	
	DWORD operation;
	DWORD auto_control;
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

//Ý_@@ŔW´_@xyFůń˛@zFN_(ăŞ+j
	Vector3 cp;		//Ý_xyz

	double R;		//˛ˇł
	double th;		//ůńpx
	double ph;		//Npx

	Vector3 cv;		//Ý_vx vy vz
	double vR;		//˛ˇłĎťŹx
	double wth;		//ůńpŹx
	double wph;		//NpŹx

//Ý×_@@ŔW´_@Ý_
	Vector3 pos;		//Ý_xyz
	double L;		//[vˇ
	double Le;		//Lř[vˇiUęüúj
	double _th;		//˝Ępx
	double _ph;		//Z˛px

	Vector3 vel;		//Ý_vx vy vz
	double vL;		// [vˇĎťŹx
	double _wth;	//˝ĘpŹx
	double _wph;	//Z˛pŹx
}ST_IO_PHYSIC, *LPST_IO_PHYSIC;

class CIO_Table
{
public:
	CIO_Table() {};
	~CIO_Table() {};

	ST_IO_PHYSIC phsic;

};
