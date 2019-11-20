#pragma once
#include "CTaskObj.h"

extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;



class CManager :	public CTaskObj
{
public:
	CManager();
	~CManager();

	void routine_work(void *param);
	CManager* pManObj;
	void init_task(void *pobj);
	LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
	bool get_UI();
	int set_job_order(DWORD type, LPST_COMMAND_TARGET p_targets);
};

