#pragma once
#include "CTaskObj.h"


extern vector<void*>	VectpCTaskObj;	//�^�X�N�I�u�W�F�N�g�̃|�C���^
extern ST_iTask g_itask;

class CPlayer :	public CTaskObj
{
public:
	CPlayer();
	~CPlayer();

	void cal_ui_order();
	void routine_work(void *param);
	void init_task(void *pobj);
};

