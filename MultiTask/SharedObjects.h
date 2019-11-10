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

#define OPE_MODE_AS_ON			0x0001
#define OPE_MODE_AS_OFF			0x0000

#define AS_PTN_DMP						0x0100
#define AS_PTN_POS						0x0200
#define AS_PTN_MOVE						0x0400
#define AS_PTN_0						0x0000

#define AS_MOVE_DEACTIVATE				0x0000
#define AS_MOVE_STANDBY					0x0001
#define AS_MOVE_LONG_SLEW				0x0002
#define AS_MOVE_POSITIONING				0x0004
#define AS_MOVE_ANTISWAY				0x0008
#define AS_MOVE_COMPLETE				0x0010
#define AS_MOVE_INTERRUPT				0x0020

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
	int antisway;			//Antisway OPE_MODE_AS_ON / OPE_MODE_AS_OFF
	int antisway_control_h;	//
	int antisway_ptn_h;		//
	int antisway_control_t;	//
	int antisway_ptn_t;		//
	int antisway_control_n;	//
	int antisway_ptn_n;		//
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

#define CONSOLE_PB_DW   	4
#define CONSOLE_PB_NUM		4
#define CONSOLE_LAMP_NUM	4
#define CONSOLE_SWITCH_NUM	8

enum CON_PBL_ID1 {
	CON_PB_VIEW1, CON_PB_VIEW2, CON_PB_VIEW3, CON_PB_VIEW4, CON_PB_VIEWUP, CON_PB_VIEWDOWN, CON_PB_7, CON_PB_AS
};
enum CON_SWITCH_ID {
	CON_SWITCH_0, CON_SWITCH_1, CON_SWITCH_2, CON_SWITCH_3, CON_SWITCH_4
};

typedef struct _stConsole {
	bool console_active;

	DWORD mh_notch_raw;		//ノッチ入力生値
	DWORD slew_notch_raw;	//ノッチ入力生値
	DWORD bh_notch_raw;		//ノッチ入力生値
	DWORD gt_notch_raw;		//ノッチ入力生値
	int mh_notch;
	int slew_notch;
	int bh_notch;
	int gt_notch;
	int mh_notch_dir;
	int slew_notch_dir;
	int bh_notch_dir;
	int gt_notch_dir;

	DWORD	PB[CONSOLE_PB_DW];
	DWORD	LAMP[CONSOLE_LAMP_NUM];
	DWORD	SWITCH[CONSOLE_SWITCH_NUM];
}ST_CONSOLE, *LPST_CONSOLE;


class CIO_Table
{
public:
	CIO_Table() {};
	~CIO_Table() {};

	ST_IO_PHYSIC	physics;
	ST_IO_REF		ref;
	ST_AS_CTRL		as_ctrl;
	ST_CONSOLE		console_ope_room;
	ST_CONSOLE		console_remote;
	ST_CONSOLE		console_pc;
};
