#pragma once
#include "CTaskObj.h"


extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;

class CPlayer :	public CTaskObj
{
public:
	CPlayer();
	~CPlayer();

	void cal_ui_order();
	void routine_work(void *param);
	void init_task(void *pobj);
};

