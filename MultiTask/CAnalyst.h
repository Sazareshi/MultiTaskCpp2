#pragma once
#include "CTaskObj.h"

extern vector<void*>	VectpCTaskObj;	//�^�X�N�I�u�W�F�N�g�̃|�C���^
extern ST_iTask g_itask;

class CAnalyst :
	public CTaskObj
{
public:
	CAnalyst();
	~CAnalyst();
};

