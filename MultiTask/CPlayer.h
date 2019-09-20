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


class CPlayer :	public CTaskObj
{
public:
	CPlayer();
	~CPlayer();

	void routine_work(void *param);
	void init_task(void *pobj);
	int auto_start(LPST_JOB_ORDER recipe, int type);
	void cal_ui_order();
	int update_mode(int order_type);//�U�~���䃂�[�h�A�������䃂�[�h�X�V
	int update_as_status();			//�U�~��ԍX�V
	int set_table_out();			//�o�̓Z�b�g
};

