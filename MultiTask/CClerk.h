#pragma once
#include "CTaskObj.h"

#include "CommonFormat.h"

extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;


class CClerk :
	public CTaskObj
{
public:
	CClerk();
	~CClerk();
};

