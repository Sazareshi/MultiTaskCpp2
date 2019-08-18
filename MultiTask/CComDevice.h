#pragma once
#include "CTaskObj.h"


extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;

class CComDevice :
	public CTaskObj
{
public:
	CComDevice();
	~CComDevice();

	void routine_work(void *param);
	void init_task(void *pobj);

};

