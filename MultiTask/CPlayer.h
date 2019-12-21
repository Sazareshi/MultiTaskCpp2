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

#define CHECK_RANGE_OF_SLEW_POS  20.0



class CPlayer :	public CTaskObj
{
public:
	CPlayer();
	~CPlayer();

	double auto_vref[MOTION_NUM];			//�U��~�߂̑��x�w�ߒl  MOTION_ID_MH, MOTION_ID_BH, MOTION_ID_SLEW
	double manual_vref[MOTION_NUM];			//�蓮�̑��x�w�ߒl  MOTION_ID_MH, MOTION_ID_BH, MOTION_ID_SLEW
	LPST_MOTION_UNIT p_motion_ptn[M_AXIS];
	ST_MOTION_UNIT motion_ptn[M_AXIS];


	int set_motion_receipe();//���s�p�^�[���̌v�Z�ƃ|�C���^�̃Z�b�g

	double act_slew_steps(LPST_MOTION_UNIT pRecipe);
	double act_bh_steps(LPST_MOTION_UNIT pRecipe);
	double act_mh_steps(LPST_MOTION_UNIT pRecipe);

	int check_step_status_slew(LPST_MOTION_ELEMENT pStep);
	int check_step_status_bh(LPST_MOTION_ELEMENT pStep);
	int check_step_status_mh(LPST_MOTION_ELEMENT pStep);

	void routine_work(void *param);
	void init_task(void *pobj);

	void cal_console_order();
	void cal_auto_ref();
	int update_auto_status();		//�U�~��ԍX�V
	int set_table_out();			//�o�̓Z�b�g



};

