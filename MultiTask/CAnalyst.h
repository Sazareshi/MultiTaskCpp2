#pragma once
#include "CTaskObj.h"
#include "Mob.h"

extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;

#define PTN_CONFIRMATION_TIME		0.1 //	Confirmation time of pattern output

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
	void cal_as_target();
	void update_as_ctrl();
	void cal_as_gain();
	int cal_as_recipe(int motion_id,ST_MOTION_UNIT* target);

	// Mob
	Mob_HoistPoint  hp;
	Mob_HungLoad	hl;
};

