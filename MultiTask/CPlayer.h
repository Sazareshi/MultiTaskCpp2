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

//### Status of 1 step
#define STEP_FIN				0
#define STEP_STANDBY			1
#define STEP_ON_GOING			2
#define STEP_PAUSE				3
#define STEP_ERROR				0xffff

#define CHECK_RANGE_OF_SLEW_POS  20.0



class CPlayer :	public CTaskObj
{
public:
	CPlayer();
	~CPlayer();

	double auto_vref[MOTION_NUM];			//振れ止めの速度指令値  MOTION_ID_MH, MOTION_ID_BH, MOTION_ID_SLEW
	double manual_vref[MOTION_NUM];			//手動の速度指令値  MOTION_ID_MH, MOTION_ID_BH, MOTION_ID_SLEW
	LPST_MOTION_UNIT p_motion_ptn[M_AXIS];
	ST_MOTION_UNIT motion_ptn[M_AXIS];


	int set_motion_receipe();//実行パターンの計算とポインタのセット

	double act_slew_steps(LPST_MOTION_UNIT pRecipe);
	double act_bh_steps(LPST_MOTION_UNIT pRecipe);
	double act_mh_steps(LPST_MOTION_UNIT pRecipe);

	int check_step_status_slew(LPST_MOTION_ELEMENT pStep);
	int check_step_status_bh(LPST_MOTION_ELEMENT pStep);
	int check_step_status_mh(LPST_MOTION_ELEMENT pStep);

	void routine_work(void *param);
	void init_task(void *pobj);

	void cal_console_order();
	void cal_auto_ref();
	int update_auto_status();		//振止状態更新
	int set_table_out();			//出力セット



};

