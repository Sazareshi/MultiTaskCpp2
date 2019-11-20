#pragma once
#include "CTaskObj.h"


extern vector<void*>	VectpCTaskObj;	//�^�X�N�I�u�W�F�N�g�̃|�C���^
extern ST_iTask g_itask;

#define ORDER_TYPE_CONST	0
#define ORDER_TYPE_MANU		1
#define ORDER_TYPE_UI		2

//�U��~�߃X�e�[�^�X
#define AS_STATUS_PROHIBITED	0x0000	// �֎~���
#define AS_STATUS_COMPLETE		0x0001	// ����
#define AS_STATUS_LOW_DEMAND	0x0002	// �����@�K�v����
#define AS_STATUS_HIGH_DEMAND	0x0004	// �����@�K�v����

#define AS_STATUS_SLEW_ENABLED  0x0100	//	�����
#define AS_STATUS_BH_ENABLED	0x0200	//	������
#define AS_STATUS_MH_ENABLED	0x0400	//	����

//### Status of 1 step
#define STEP_FIN				0
#define STEP_STANDBY			1
#define STEP_ON_GOING			2
#define STEP_PAUSE				3
#define STEP_ERROR				0xffff

class CPlayer :	public CTaskObj
{
public:
	CPlayer();
	~CPlayer();

	double auto_vref[MOTION_NUM];	//�U��~�߂̑��x�w�ߒl  MOTION_ID_MH, MOTION_ID_BH, MOTION_ID_SLEW
	double manual_vref[MOTION_NUM];	//�蓮�̑��x�w�ߒl  MOTION_ID_MH, MOTION_ID_BH, MOTION_ID_SLEW
	
	
	LPST_MOTION_UNIT p_bh_motion_ptn;
	LPST_MOTION_UNIT p_slew_motion_ptn;
	LPST_MOTION_UNIT p_mh_motion_ptn;

	ST_MOTION_UNIT bh_motion_ptn;
	ST_MOTION_UNIT slew_motion_ptn;
	ST_MOTION_UNIT mh_motion_ptn;

	double act_slew_steps(LPST_MOTION_UNIT pRecipe);
	double act_bh_steps(LPST_MOTION_UNIT pRecipe);
	double act_mh_steps(LPST_MOTION_UNIT pRecipe);
	

	int set_motion_ptn_unit();

	int check_step_status_slew(LPST_MOTION_ELEMENT pStep);
	int check_step_status_bh(LPST_MOTION_ELEMENT pStep);
	int check_step_status_mh(LPST_MOTION_ELEMENT pStep);


	void routine_work(void *param);
	void init_task(void *pobj);
	int auto_start(LPST_JOB_ORDER recipe, int type);
	void cal_console_order();
	void cal_auto_ref();
	int update_as_status();			//�U�~��ԍX�V
	int set_table_out();			//�o�̓Z�b�g
};

