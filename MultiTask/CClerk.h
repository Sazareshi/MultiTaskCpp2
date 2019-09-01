#pragma once
#include "CTaskObj.h"
#include "CManager.h"

extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;


class CClerk :
	public CTaskObj
{
public:
	CClerk();
	~CClerk();

	CClerk* pClkObj;

	LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
	void set_panel_tip_txt();//タブパネルのStaticテキストを設定
	void routine_work(void *param);

	void init_task(void *pobj);
	HWND CreateOwnWindow(HWND h_parent_wnd);
	BOOL InitWorkWnd(HINSTANCE hInst, WNDPROC WndProc, LPCTSTR lpzClassName);

	static LRESULT CALLBACK ChartWndProc(HWND, UINT, WPARAM, LPARAM);


};

