#include "stdafx.h"
#include "CManager.h"
#include "CPublicRelation.h"
#include "CPlayer.h"
#include "CAnalyst.h"

extern CMODE_Table*	pMode;				//共有メモリModeクラスポインタ
extern CORDER_Table* pOrder;			//共有メモリOrderクラスポインタ

CManager::CManager(){
	pManObj = this;
}

CManager::~CManager(){}


bool CManager::get_UI() {
	pMode->environment = pOrder->ui.env_mode;
	pMode->operation = pOrder->ui.ope_mode;
	pMode->antisway = pOrder->ui.as_mode;

	if (pOrder->ui.anti_sway_trigger == ON) {
		if (pMode->antisway == OPE_MODE_AS_ON) pMode->antisway = OPE_MODE_AS_OFF;
		else pMode->antisway = OPE_MODE_AS_ON;
	}

	if (pOrder->ui.auto_mode == AUTO_MODE_ACTIVE) {
		CAnalyst* pAna = (CAnalyst*)VectpCTaskObj[g_itask.ana];
		pAna->cal_job_recipe(pOrder->ui.auto_mode);

		CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
		pPly->auto_start(&(pOrder->job_A),pOrder->ui.auto_mode);
	}


	return FALSE;
}
void CManager::init_task(void *pobj) {
	set_panel_tip_txt();
	pMode->environment	= ENV_MODE_SIM1;
	pMode->operation	= OPE_MODE_MANUAL;
	pMode->auto_control = AUTO_MODE_STANDBY;
	pMode->antisway = OPE_MODE_AS_OFF;
};

void CManager::routine_work(void *param) {
	ws << L" working!" << *(inf.psys_counter)%100 << L" Env=" << pMode->environment << L" Ope=" << pMode->operation << L" AS=" << pMode->antisway << L" AUTO=" << pMode->auto_control;
	tweet2owner(ws.str()); ws.str(L""); ws.clear();
};

LRESULT CALLBACK CManager::PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {

	switch (msg) {

	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDC_TASK_FUNC_RADIO1:
		case IDC_TASK_FUNC_RADIO2:
		case IDC_TASK_FUNC_RADIO3:
		case IDC_TASK_FUNC_RADIO4:
		case IDC_TASK_FUNC_RADIO5:
		case IDC_TASK_FUNC_RADIO6:
			inf.panel_func_id = LOWORD(wp); set_panel_tip_txt(); set_PNLparam_value(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
			break;

		case IDC_TASK_ITEM_RADIO1:
		case IDC_TASK_ITEM_RADIO2:
		case IDC_TASK_ITEM_RADIO3:
		case IDC_TASK_ITEM_RADIO4:
		case IDC_TASK_ITEM_RADIO5:
		case IDC_TASK_ITEM_RADIO6:
			inf.panel_type_id = LOWORD(wp); set_panel_tip_txt();  SetFocus(GetDlgItem(inf.hWnd_opepane, IDC_TASK_EDIT1));
			break;

		case IDSET: {
			wstring wstr, wstr_tmp;

			//サンプルとしていろいろな型で読み込んで表示している
			wstr += L"Param 1(d):";
			int n = GetDlgItemText(hDlg, IDC_TASK_EDIT1, (LPTSTR)wstr_tmp.c_str(), 128);
			if (n) wstr_tmp = to_wstring(stod(wstr_tmp));	wstr = wstr + wstr_tmp.c_str();

			wstr += L",  Param 2(i):";
			n = GetDlgItemText(hDlg, IDC_TASK_EDIT2, (LPTSTR)wstr_tmp.c_str(), 128);
			if (n) wstr_tmp = to_wstring(stoi(wstr_tmp));	wstr = wstr + wstr_tmp.c_str();

			wstr += L",  Param 3(f):";
			n = GetDlgItemText(hDlg, IDC_TASK_EDIT3, (LPTSTR)wstr_tmp.c_str(), 128);
			if (n) wstr_tmp = to_wstring(stof(wstr_tmp));	wstr = wstr + wstr_tmp.c_str();

			wstr += L",  Param 4(l):";
			n = GetDlgItemText(hDlg, IDC_TASK_EDIT4, (LPTSTR)wstr_tmp.c_str(), 128);
			if (n) wstr_tmp = to_wstring(stol(wstr_tmp));	wstr = wstr + wstr_tmp.c_str();

			wstr += L",  Param 5(c):";
			n = GetDlgItemText(hDlg, IDC_TASK_EDIT5, (LPTSTR)wstr_tmp.c_str(), 128);
			wstr += wstr_tmp.c_str();

			wstr += L",   Param 6(c):";
			n = GetDlgItemText(hDlg, IDC_TASK_EDIT6, (LPTSTR)wstr_tmp.c_str(), 128);
			wstr += wstr_tmp.c_str();

			txout2msg_listbox(wstr);


		}break;
		case IDRESET: {
			set_PNLparam_value(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

		}break;

		case IDC_TASK_OPTION_CHECK1:
			SendMessage(GetDlgItem(hDlg, IDC_TASK_OPTION_CHECK2), BM_SETCHECK, BST_UNCHECKED, 0L);
			if (IsDlgButtonChecked(hDlg, IDC_TASK_OPTION_CHECK1) == BST_CHECKED) inf.work_select = THREAD_WORK_OPTION1;
			else inf.work_select = THREAD_WORK_ROUTINE;
			break;

		case IDC_TASK_OPTION_CHECK2:
			SendMessage(GetDlgItem(hDlg, IDC_TASK_OPTION_CHECK1), BM_SETCHECK, BST_UNCHECKED, 0L);
			if (IsDlgButtonChecked(hDlg, IDC_TASK_OPTION_CHECK2) == BST_CHECKED) inf.work_select = THREAD_WORK_OPTION2;
			else inf.work_select = THREAD_WORK_ROUTINE;
			break;
		}
		break;
	}
	return 0;
};