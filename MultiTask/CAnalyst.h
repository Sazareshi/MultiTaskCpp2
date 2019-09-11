#pragma once
#include "CTaskObj.h"
#include "Mob.h"

extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;

class CAnalyst :
	public CTaskObj
{
public:
	CAnalyst();
	~CAnalyst();

	void init_task(void *pobj);
	void routine_work(void *param);
	void cal_simulation();
	LPST_JOB_ORDER cal_job_recipe(int job_type);

	// Mob
	Mob_HoistPoint  hp;
	Mob_HungLoad	hl;
};

