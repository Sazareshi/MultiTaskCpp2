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

	int ptn_notch_freq[NUM_OF_AS][NOTCH_MAX];
	void init_task(void *pobj);
	void routine_work(void *param);
	void cal_simulation();
	LPST_JOB_ORDER cal_job_recipe(int job_type);
	void cal_as_target();
	void update_as_ctrl();
	void cal_as_gain();
	int cal_as_recipe(int motion_id,LPST_MOTION_UNIT target, int mode);
	int cal_long_move_recipe(int motion_id, LPST_MOTION_UNIT target, int mode);
	int cal_short_move_recipe(int motion_id, LPST_MOTION_UNIT target, int mode);
	int cal_move_pattern_bh(int * notch_freq, double Da, int mode);
	int cal_move_pattern_slew(int * notch_freq, double Da, int mode);

	// Mob
	Mob_HoistPoint  hp;
	Mob_HungLoad	hl;
};

