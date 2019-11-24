#pragma once
#include "CTaskObj.h"

extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;

#define JOB_EVENT_ACTIVATE_COMMAND_STEP					1
#define JOB_EVENT_COMPLETE_COMMAND_STEP_NORMAL			2
#define JOB_EVENT_COMPLETE_COMMAND_STEP_ABNORMAL		3

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
	int handle_order_event(int order_id, int event_id, int option);
};

