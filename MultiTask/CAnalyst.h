#pragma once
#include "CTaskObj.h"
#include "Mob.h"

extern vector<void*>	VectpCTaskObj;	//�^�X�N�I�u�W�F�N�g�̃|�C���^
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

typedef struct AS_notch {//�U��~�ߑ��x�ݒ�p
	int i_notch_top[NUM_OF_AS];//TOP SPEED�̃m�b�`
	int i_notch_2nd[NUM_OF_AS];//2�i�ڂ̃m�b�`
	double v_top[NUM_OF_AS];//TOP SPEED
	double v_2nd[NUM_OF_AS];//2�i��SPEED
	double t_acc_top[NUM_OF_AS];//2�i�ڂ���TOP SPEED�܂ł̉�������
	double t_acc_2nd[NUM_OF_AS];//2�i�ڂ܂ł̉�������
	double t_const_d[NUM_OF_AS];//�����������ԁi�g�b�v�X�s�[�h�葬���ԁj
	double t_const_2nd[NUM_OF_AS];//2�i���������̉��������U��~�ߒ葬����
} AS_NOTCH_SET, *LPAS_NOTCH_SET;



class CAnalyst :
	public CTaskObj
{
public:
	CAnalyst();
	~CAnalyst();


	AS_NOTCH_SET as_notch_spd;
		
	void init_task(void *pobj);
	void routine_work(void *param);
	void cal_simulation();
	void cal_auto_target(int mode);
	void update_auto_ctrl();
	void cal_as_gain(int motion_id,int type);

	int cal_job_recipe(int job_id, int mode);
	int cal_move_1Step(int motion_id,LPST_MOTION_UNIT target, int mode);
	int cal_move_2Step_pn(int motion_id, LPST_MOTION_UNIT target, int mode);
	int cal_move_2Step_pp(int motion_id, LPST_MOTION_UNIT target, int mode);
	int cal_move_3Step(int motion_id, LPST_MOTION_UNIT target, int mode);
	int cal_move_2accdec(int motion_id, LPST_MOTION_UNIT target, int mode);
	int cal_move_trapezoid(int motion_id, LPST_MOTION_UNIT target, int mode);

	int cal_notch_set(LPAS_NOTCH_SET notch_set, double Da, int mode, int type);

	// Mob
	Mob_HoistPoint  hp;
	Mob_HungLoad	hl;

private:
	double buf_average_phase_rdz[MAX_SAMPLE_AVERAGE_RZ];
	int i_buf_rdz;

};

