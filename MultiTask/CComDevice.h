#pragma once
#include "CTaskObj.h"


extern vector<void*>	VectpCTaskObj;	//�^�X�N�I�u�W�F�N�g�̃|�C���^
extern ST_iTask g_itask;

class CComDevice :
	public CTaskObj
{
public:
	CComDevice();
	~CComDevice();

	void routine_work(void *param);
	void init_task(void *pobj);

	LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
	void set_panel_tip_txt();//�^�u�p�l����Static�e�L�X�g��ݒ�

	void update_ope_room_consol();
	void update_remote_room_consol();
};

