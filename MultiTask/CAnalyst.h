#pragma once
#include "CTaskObj.h"
#include "Mob.h"

extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;

#define PTN_CONFIRMATION_TIME		0.1 //	Confirmation time of pattern output
#define PTN_ERROR_CHECK_TIME1		60 //	Confirmation time of pattern output
#define PTN_HOIST_ADJUST_TIME		0.15 //	Confirmation time of pattern output

#define CAL_FOR_AUTO_JOB_A			1
#define CAL_FOR_AUTO_JOB_B			2
#define CAL_FOR_MANUAL				3

#define NUM_OF_2STEP				3
#define DEPTH_OF_2STEP_BH			3
#define DEPTH_OF_2STEP_SLEW			3

#define MAX_SAMPLE_AVERAGE_RZ       3

typedef struct i2stepV {
	int iV1[NUM_OF_2STEP];
	int iV2[NUM_OF_2STEP];
	double v1;
	double v2;
} I2STP_V, *LPI2STP_V;



class CAnalyst :
	public CTaskObj
{
public:
	CAnalyst();
	~CAnalyst();


	I2STP_V i_vlong_slew;
	I2STP_V i_vlong_bh;

	int ptn_notch_freq[NUM_OF_AS][NOTCH_MAX];
	void init_task(void *pobj);
	void routine_work(void *param);
	void cal_simulation();
	void cal_auto_target(int mode);
	void update_auto_ctrl();
	void cal_as_gain();
	int cal_job_recipe(int job_id, int mode);
	int cal_as_recipe(int motion_id,LPST_MOTION_UNIT target, int mode);
	int cal_long_move_recipe(int motion_id, LPST_MOTION_UNIT target, int mode);
	int cal_long_move_recipe2(int motion_id, LPST_MOTION_UNIT target, int mode);
	int cal_short_move_recipe(int motion_id, LPST_MOTION_UNIT target, int mode);
	int cal_short_move_recipe2(int motion_id, LPST_MOTION_UNIT target, int mode);
	int cal_move_pattern_bh(int * notch_freq, double Da, int mode);
	int cal_move_pattern_slew(int * notch_freq, double Da, int mode);

	// Mob
	Mob_HoistPoint  hp;
	Mob_HungLoad	hl;

private:
	double buf_average_phase_rdz[MAX_SAMPLE_AVERAGE_RZ];
	int i_buf_rdz;

};

