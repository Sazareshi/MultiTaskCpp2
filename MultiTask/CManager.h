#pragma once
#include "CTaskObj.h"
#include "CommonFormat.h"

extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;



class CManager :	public CTaskObj
{
public:
	CManager();
	~CManager();
};

