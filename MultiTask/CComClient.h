#pragma once
#include "CTaskObj.h"
#include "CManager.h"


extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;


class CComClient :
	public CTaskObj
{
public:
	CComClient();
	~CComClient();

	LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
	void set_panel_tip_txt();//タブパネルのStaticテキストを設定
	void routine_work(void *param);

};

