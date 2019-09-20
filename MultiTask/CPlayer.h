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


class CPlayer :	public CTaskObj
{
public:
	CPlayer();
	~CPlayer();

	void routine_work(void *param);
	void init_task(void *pobj);
	int auto_start(LPST_JOB_ORDER recipe, int type);
	void cal_ui_order();
	int update_mode(int order_type);//振止制御モード、自動制御モード更新
	int update_as_status();			//振止状態更新
	int set_table_out();			//出力セット
};

