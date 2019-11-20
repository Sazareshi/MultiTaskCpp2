#include "stdafx.h"
#include "CComDevice.h"
#include "CPublicRelation.h"
#include "Helper.h"

extern CORDER_Table*	pOrder;
extern CMODE_Table*	pMode;
extern CIO_Table*	pIO_Table;

CComDevice::CComDevice(){}

CComDevice::~CComDevice(){}


void CComDevice::init_task(void *pobj) {
	set_panel_tip_txt();
	return;
};

void CComDevice::routine_work(void *param) {
/*
	JOYINFOEX JoyInfoEx;
	JoyInfoEx.dwSize = sizeof(JOYINFOEX);
	JoyInfoEx.dwFlags = JOY_RETURNALL;

	if (JOYERR_NOERROR == joyGetPosEx(0, &JoyInfoEx)) { //0番のジョイスティックの情報を見る
		ws <<  L" working!" << *(inf.psys_counter) % 100 <<" :dwXpos = " << JoyInfoEx.dwXpos << "  dwYpos = " << JoyInfoEx.dwYpos << "  dwZpos = " << JoyInfoEx.dwRpos  << "  dwButton = " << JoyInfoEx.dwButtons;
	}
	else {
		ws << L" working!" << *(inf.psys_counter) % 100 << "JOY STICK Error";
	}
	*/
	//ws << L" working!" << *(inf.psys_counter)%100;
	update_remote_room_consol();
	tweet2owner(ws.str()); ws.str(L""); ws.clear();

};

void CComDevice::update_remote_room_consol() {

	JOYINFOEX JoyInfoEx;
	JoyInfoEx.dwSize = sizeof(JOYINFOEX);
	JoyInfoEx.dwFlags = JOY_RETURNALL;

	if (JOYERR_NOERROR == joyGetPosEx(0, &JoyInfoEx)) { //0番のジョイスティックの情報を見る
		pIO_Table->console_remote.console_active = true;

		pIO_Table->console_remote.bh_notch_raw = JoyInfoEx.dwYpos;
		pIO_Table->console_remote.slew_notch_raw = JoyInfoEx.dwXpos;
		pIO_Table->console_remote.mh_notch_raw = JoyInfoEx.dwRpos;
		pIO_Table->console_remote.PB[0] = JoyInfoEx.dwButtons;
		pIO_Table->console_remote.SWITCH[0] = JoyInfoEx.dwPOV;

		int tempi = pIO_Table->console_remote.bh_notch_raw ;
		if (tempi < 0x1FFF) {
			pIO_Table->console_remote.bh_notch_dir = 1;  pIO_Table->console_remote.bh_notch = 5;
		}
		else if(tempi < 0x3FFF){
			pIO_Table->console_remote.bh_notch_dir = 1;  pIO_Table->console_remote.bh_notch = 3;
		}
		else if (tempi < 0x6FFF) {
			pIO_Table->console_remote.bh_notch_dir = 1;  pIO_Table->console_remote.bh_notch = 1;
		}
		else if (tempi < 0x8FFF) {
			pIO_Table->console_remote.bh_notch_dir = 0;  pIO_Table->console_remote.bh_notch = 0;
		}
		else if (tempi < 0xAFFF) {
			pIO_Table->console_remote.bh_notch_dir = -1;  pIO_Table->console_remote.bh_notch = 1;
		}
		else if (tempi < 0xDFFF) {
			pIO_Table->console_remote.bh_notch_dir = -1;  pIO_Table->console_remote.bh_notch = 3;
		}
		else{
			pIO_Table->console_remote.bh_notch_dir = -1;  pIO_Table->console_remote.bh_notch = 5;
		}

		tempi = pIO_Table->console_remote.slew_notch_raw;
		if (tempi < 0x1FFF) {
			pIO_Table->console_remote.slew_notch_dir = -1;  pIO_Table->console_remote.slew_notch = 5;
		}
		else if (tempi < 0x3FFF) {
			pIO_Table->console_remote.slew_notch_dir = -1;  pIO_Table->console_remote.slew_notch = 3;
		}
		else if (tempi < 0x6FFF) {
			pIO_Table->console_remote.slew_notch_dir = -1;  pIO_Table->console_remote.slew_notch = 1;
		}
		else if (tempi < 0x8FFF) {
			pIO_Table->console_remote.slew_notch_dir = 0;  pIO_Table->console_remote.slew_notch = 0;
		}
		else if (tempi < 0xAFFF) {
			pIO_Table->console_remote.slew_notch_dir = 1;  pIO_Table->console_remote.slew_notch = 1;
		}
		else if (tempi < 0xDFFF) {
			pIO_Table->console_remote.slew_notch_dir = 1;  pIO_Table->console_remote.slew_notch = 3;
		}
		else {
			pIO_Table->console_remote.slew_notch_dir = 1;  pIO_Table->console_remote.slew_notch = 5;
		}

		tempi = pIO_Table->console_remote.mh_notch_raw;
		if (tempi < 0x1FFF) {
			pIO_Table->console_remote.mh_notch_dir = 1;  pIO_Table->console_remote.mh_notch = 5;
		}
		else if (tempi < 0x3FFF) {
			pIO_Table->console_remote.mh_notch_dir = 1;  pIO_Table->console_remote.mh_notch = 3;
		}
		else if (tempi < 0x6FFF) {
			pIO_Table->console_remote.mh_notch_dir = 1;  pIO_Table->console_remote.mh_notch = 1;
		}
		else if (tempi < 0x8FFF) {
			pIO_Table->console_remote.mh_notch_dir = 0;  pIO_Table->console_remote.mh_notch = 0;
		}
		else if (tempi < 0xAFFF) {
			pIO_Table->console_remote.mh_notch_dir = -1;  pIO_Table->console_remote.mh_notch = 1;
		}
		else if (tempi < 0xDFFF) {
			pIO_Table->console_remote.mh_notch_dir = -1;  pIO_Table->console_remote.mh_notch = 3;
		}
		else {
			pIO_Table->console_remote.mh_notch_dir = -1;  pIO_Table->console_remote.mh_notch = 5;
		}
	
		tempi = pIO_Table->console_remote.gt_notch_raw;
		if (tempi < 0x1FFF) {
			pIO_Table->console_remote.gt_notch_dir = -1;  pIO_Table->console_remote.gt_notch = 5;
		}
		else if (tempi < 0x3FFF) {
			pIO_Table->console_remote.gt_notch_dir = -1;  pIO_Table->console_remote.gt_notch = 3;
		}
		else if (tempi < 0x6FFF) {
			pIO_Table->console_remote.gt_notch_dir = -1;  pIO_Table->console_remote.gt_notch = 1;
		}
		else if (tempi < 0x8FFF) {
			pIO_Table->console_remote.gt_notch_dir = 0;  pIO_Table->console_remote.gt_notch = 0;
		}
		else if (tempi < 0xAFFF) {
			pIO_Table->console_remote.gt_notch_dir = 1;  pIO_Table->console_remote.gt_notch = 1;
		}
		else if (tempi < 0xDFFF) {
			pIO_Table->console_remote.gt_notch_dir = 1;  pIO_Table->console_remote.gt_notch = 3;
		}
		else {
			pIO_Table->console_remote.gt_notch_dir = 1;  pIO_Table->console_remote.gt_notch = 5;
		}

		ws << *(inf.psys_counter) % 100 << " :dwXpos = " << JoyInfoEx.dwXpos << "  dwYpos = " << JoyInfoEx.dwYpos << "  dwZpos = " << JoyInfoEx.dwRpos << "  dwButton = " << JoyInfoEx.dwButtons << "  dwPOV = " << JoyInfoEx.dwPOV;
	}
	else {
		pIO_Table->console_remote.console_active = false;
		ws << L" working!" << *(inf.psys_counter) % 100 << "  JOY STICK Error";
	}


	if(Bitcheck(pIO_Table->console_remote.PB[0],CON_PB_AS)){
		if (pOrder->ui.as_mode == OPE_MODE_AS_OFF)pOrder->ui.as_mode = OPE_MODE_AS_ON;
		else pOrder->ui.as_mode = OPE_MODE_AS_OFF;
		CManager* pMan = (CManager*)VectpCTaskObj[g_itask.mng];
		pMan->get_UI();
	}
}

LRESULT CALLBACK CComDevice::PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {

	CManager* pMan = (CManager*)VectpCTaskObj[g_itask.mng];

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
			if (inf.panel_func_id == IDC_TASK_FUNC_RADIO1) {
				if (inf.panel_type_id == IDC_TASK_ITEM_RADIO1) {
					pIO_Table->console_pc.slew_notch = 5;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO2) {
					pIO_Table->console_pc.slew_notch = 0;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO3) {
					pIO_Table->console_pc.slew_notch = -5;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO4) {
					pIO_Table->console_pc.bh_notch = 5;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO5) {
					pIO_Table->console_pc.bh_notch = 0;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO6) {
					pIO_Table->console_pc.bh_notch = -1;
				}
				else;
			}
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

void CComDevice::set_panel_tip_txt()
{
	wstring wstr_type; wstring wstr;
	switch (inf.panel_func_id) {
	case IDC_TASK_FUNC_RADIO1: {
		wstr = L"Type for Func1 \n\r 1:Slew- 2:0 3:Slew+ \n\r 4:Bh- 5:0 6:Bh+";
		switch (inf.panel_type_id) {
		case IDC_TASK_ITEM_RADIO1:
			wstr_type += L"Param of type1 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO2:
			wstr_type += L"Param of type2 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO3:
			wstr_type += L"Param of type3 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO4:
			wstr_type += L"Param of type4 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO5:
			wstr_type += L"Param of type5 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO6:
			wstr_type += L"Param of type6 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		default:break;
		}
	}break;
	case IDC_TASK_FUNC_RADIO2: {
		wstr = L"Type of Func2 \n\r 1:?? 2:?? 3:?? \n\r 4:?? 5:?? 6:??";
		switch (inf.panel_type_id) {
		case IDC_TASK_ITEM_RADIO1:
			wstr_type += L"Param of type1 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO2:
			wstr_type += L"Param of type2 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO3:
			wstr_type += L"Param of type3 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO4:
			wstr_type += L"Param of type4 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO5:
			wstr_type += L"Param of type5 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO6:
			wstr_type += L"Param of type6 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		default:break;
		}
	}break;
	case IDC_TASK_FUNC_RADIO3: {
		wstr = L"Type for Func3 \n\r 1:?? 2:?? 3:?? \n\r 4:?? 5:?? 6:??";
		switch (inf.panel_type_id) {
		case IDC_TASK_ITEM_RADIO1:
			wstr_type += L"Param of type1 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO2:
			wstr_type += L"Param of type2 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO3:
			wstr_type += L"Param of type3 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO4:
			wstr_type += L"Param of type4 \n\r 1:AS_ACT 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO5:
			wstr_type += L"Param of type5 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO6:
			wstr_type += L"Param of type6 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		default:break;
		}
	}break;
	case IDC_TASK_FUNC_RADIO4: {
		wstr = L"Type for Func4 \n\r 1:?? 2:?? 3:?? \n\r 4:?? 5:?? 6:??";
		switch (inf.panel_type_id) {
		case IDC_TASK_ITEM_RADIO1:
			wstr_type += L"Param of type1 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO2:
			wstr_type += L"Param of type2 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO3:
			wstr_type += L"Param of type3 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO4:
			wstr_type += L"Param of type4 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO5:
			wstr_type += L"Param of type5 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO6:
			wstr_type += L"Param of type6 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		default:break;
		}
	}break;
	case IDC_TASK_FUNC_RADIO5: {
		wstr = L"Type for Func5 \n\r 1:?? 2:?? 3:?? \n\r 4:?? 5:?? 6:??";
		switch (inf.panel_type_id) {
		case IDC_TASK_ITEM_RADIO1:
			wstr_type += L"Param of type1 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO2:
			wstr_type += L"Param of type2 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO3:
			wstr_type += L"Param of type3 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO4:
			wstr_type += L"Param of type4 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO5:
			wstr_type += L"Param of type5 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO6:
			wstr_type += L"Param of type6 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		default:break;
		}
	}break;
	case IDC_TASK_FUNC_RADIO6: {
		wstr = L"Func6(Mode) \n\r 1:?? 2:?? 3:?? \n\r 4:?? 5:?? 6:??";
		switch (inf.panel_type_id) {
		case IDC_TASK_ITEM_RADIO1:
			wstr_type += L"Param of type1 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO2:
			wstr_type += L"Param of type2 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO3:
			wstr_type += L"Param of type3 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO4:
			wstr_type += L"Param of type4 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO5:
			wstr_type += L"Param of type5 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO6:
			wstr_type += L"Param of type6 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		default:break;
		}
	}break;
	default: {
		wstr = L"Type for Func? \n\r 1:?? 2:?? 3:?? \n\r 4:?? 5:?? 6:??";
		wstr_type += L"(Param of type?) \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
	}break;
	}

	SetWindowText(GetDlgItem(inf.hWnd_opepane, IDC_STATIC_TASKSET3), wstr.c_str());
	SetWindowText(GetDlgItem(inf.hWnd_opepane, IDC_STATIC_TASKSET4), wstr_type.c_str());
}


