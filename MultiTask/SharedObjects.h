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

#define OPE_MODE_MANUAL			0x0000
#define OPE_MODE_REMOTE_MANUAL	0x0001
#define OPE_MODE_AUTO_ENABLE	0x0003

#define OPE_MODE_AS_ON	0x0001
#define OPE_MODE_AS_OFF	0x0000

#define AS_MODE_DEACTIVATE		0x0000
#define AS_MODE_STANDBY			0x0001
#define AS_MODE_ACTIVE			0x0004

#define AUTO_MODE_DEACTIVATE	0x0000
#define AUTO_MODE_STANDBY		0x0001
#define AUTO_MODE_SUSPEND		0x0002
#define AUTO_MODE_INTERRUPT		0x0003
#define AUTO_MODE_ACTIVE		0x0004



class CMODE_Table
{
public:
	CMODE_Table() {};
	~CMODE_Table() {};
	
	int operation;			//AUTO MANUAL 
	int antisway;			//Antisway ON / OFF
	int antisway_control;	//
	int auto_control;
	int environment;
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


/************************************************/
/*    IO TABLE		 L                          */
/************************************************/
class CIO_Table
{
public:
	CIO_Table() {};
	~CIO_Table() {};

	ST_IO_PHYSIC	physics;
	ST_IO_REF		ref;
	ST_AS_CTRL		as_ctrl;

};
