#pragma once
#include "CTaskObj.h"


extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;

#define ORDER_TYPE_CONST	0
#define ORDER_TYPE_MANU		1
#define ORDER_TYPE_UI		2

//振れ止めステータス
#define AS_STATUS_PROHIBITED	0x0000	// 禁止状態
#define AS_STATUS_COMPLETE		0x0001	// 完了
#define AS_STATUS_LOW_DEMAND	0x0002	// 未完　必要性低
#define AS_STATUS_HIGH_DEMAND	0x0004	// 未完　必要性高

#define AS_STATUS_SLEW_ENABLED  0x0100	//	旋回可
#define AS_STATUS_BH_ENABLED	0x0200	//	引込可
#define AS_STATUS_MH_ENABLED	0x0400	//	巻可

//### Array of motion
#define PLY_NUM_OF_MOTION_COUNTER 3
#define PLY_ID_DEFAULT_MOVE 0
#define PLY_ID_AUTO_MOVE 1
#define PLY_ID_AS_MOVE	2

//### Status of 1 step
#define STEP_FIN				0
#define STEP_STANDBY			1
#define STEP_ON_GOING			2
#define STEP_PAUSE				3

class CPlayer :	public CTaskObj
{
public:
	CPlayer();
	~CPlayer();

	double as_vref[MOTION_NUM];				//振れ止めの速度指令値  MOTION_ID_MH, MOTION_ID_BH, MOTION_ID_SLEW
	double manual_vref[MOTION_NUM];			//手動の速度指令値  MOTION_ID_MH, MOTION_ID_BH, MOTION_ID_SLEW
	
	int bh_ptn_count[PLY_NUM_OF_MOTION_COUNTER];
	int slew_ptn_count[PLY_NUM_OF_MOTION_COUNTER];
	int mh__ptn_count[PLY_NUM_OF_MOTION_COUNTER];
		
	ST_MOTION_ELEMENT bh_one_step[PLY_NUM_OF_MOTION_COUNTER];
	ST_MOTION_ELEMENT slew_one_step[PLY_NUM_OF_MOTION_COUNTER];
	ST_MOTION_ELEMENT mh_one_step[PLY_NUM_OF_MOTION_COUNTER];

	double act_one_step_slew(int mode, ST_MOTION_ELEMENT* recipe);
	double act_one_step_bh(int mode, ST_MOTION_ELEMENT* recipe);
	double act_one_step_mh(int mode, ST_MOTION_ELEMENT* recipe);
		
	void routine_work(void *param);
	void init_task(void *pobj);
	int auto_start(LPST_JOB_ORDER recipe, int type);
	void cal_ui_order();
	void cal_as_ref();
	int update_mode(int order_type);//振止制御モード、自動制御モード更新
	int update_as_status();			//振止状態更新
	int set_table_out();			//出力セット
};

