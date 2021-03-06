#include "stdafx.h"
#include "CPublicRelation.h"
#include <stdio.h>

extern CORDER_Table*	pOrder;				//共有メモリOrderクラスポインタ
extern CMODE_Table*		pMode;				//共有メモリModeクラスポインタ
extern ST_SPEC			g_spec;				//クレーン仕様
extern CIO_Table*		pIO_Table;

CPublicRelation::CPublicRelation(){
	st_gl_basic.bGLactive = FALSE;
	pPRObj = this;
}

CPublicRelation::~CPublicRelation(){}

ST_GL_BASIC CPublicRelation::st_gl_basic; // OpenGL 基本構造体



void CPublicRelation::routine_work(void *param) {

	check_console();
	if (st_gl_basic.bGLactive) glutPostRedisplay(); //glutDisplayFunc()を１回実行する

	ws <<  L" working!"<< *(inf.psys_counter)%100 ;
	tweet2owner(ws.str()); ws.str(L""); ws.clear();

};

void CPublicRelation::check_console() {
	if (pIO_Table->console_remote.console_active == true) {
		if (pIO_Table->console_remote.PB[0] & 0x003f) {
			if (Bitcheck(pIO_Table->console_remote.PB[0], CON_PB_VIEW1)) {
				VP_mode = 1;
				st_gl_basic.fovy = 120.0;
			}
			else if (Bitcheck(pIO_Table->console_remote.PB[0], CON_PB_VIEW2)) {
				VP_mode = 2;
				st_gl_basic.ViewPoint = pIO_Table->physics.cp;
				st_gl_basic.ViewPoint.z += 10.0;

				st_gl_basic.ViewCenter = pIO_Table->physics.cp;
				st_gl_basic.ViewCenter.x += 0.1;
				st_gl_basic.ViewCenter.z = 0.0;

				st_gl_basic.ViewUpside.x = sin(pIO_Table->physics.th);
				st_gl_basic.ViewUpside.y = cos(pIO_Table->physics.th);
				st_gl_basic.ViewUpside.z = 0.0;

				st_gl_basic.fovy = 30.0;
			}
			else if (Bitcheck(pIO_Table->console_remote.PB[0], CON_PB_VIEW3)) {
				VP_mode = 3;
				st_gl_basic.ViewPoint.x = 10.0;
				st_gl_basic.ViewPoint.y = 50.0;
				st_gl_basic.ViewPoint.z = 100.0;

				st_gl_basic.VP_Offset.x = 0.0;
				st_gl_basic.VP_Offset.y = 0.0;
				st_gl_basic.VP_Offset.z = 0.0;


				st_gl_basic.ViewCenter.x = 0.0;
				st_gl_basic.ViewCenter.y = 0.0;
				st_gl_basic.ViewCenter.z = 20.0;

				st_gl_basic.ViewUpside.x = 0.0;
				st_gl_basic.ViewUpside.y = 0.0;
				st_gl_basic.ViewUpside.z = 1.0;

				st_gl_basic.fovy = 60.0;
				st_gl_basic.aspect = (double)st_gl_basic.WinWidth / (double)st_gl_basic.WinHeight;
				st_gl_basic.zNear = 0.1;
				st_gl_basic.zFar = 900.0;
			}
			else if (Bitcheck(pIO_Table->console_remote.PB[0], CON_PB_VIEW4)) {
				VP_mode = 4;
				st_gl_basic.ViewPoint.x = 0.0;
				st_gl_basic.ViewPoint.y = 1.0;
				st_gl_basic.ViewPoint.z = 100.0;

				st_gl_basic.ViewCenter.x = 0.0;
				st_gl_basic.ViewCenter.y = 0.1;
				st_gl_basic.ViewCenter.z = 0.0;

				st_gl_basic.ViewUpside.x = 0.0;
				st_gl_basic.ViewUpside.y = 0.0;
				st_gl_basic.ViewUpside.z = 1.0;

				st_gl_basic.fovy = 60.0;
			}
			else if (Bitcheck(pIO_Table->console_remote.PB[0], CON_PB_VIEWUP)) {
				st_gl_basic.ViewPoint.z += 5.0;
			}
			else if (Bitcheck(pIO_Table->console_remote.PB[0], CON_PB_VIEWDOWN)) {
				st_gl_basic.ViewPoint.z -= 5.0;
			}
			else;
		}

		if (pIO_Table->console_remote.SWITCH[0] != 0xffff) {
			if (pIO_Table->console_remote.SWITCH[0] == 0) st_gl_basic.fovy += 1.0;
			else if (pIO_Table->console_remote.SWITCH[0] == 18000) st_gl_basic.fovy -= 1.0;
			else;

		}
	}
	;
}

LRESULT CALLBACK CPublicRelation::PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {

	CManager* pMan = (CManager*)VectpCTaskObj[g_itask.mng];

	switch (msg) {
	
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDC_TASK_FUNC_RADIO1:
		{
			inf.panel_func_id = LOWORD(wp); set_panel_tip_txt(); set_PNLparam_value(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
			if(st_gl_basic.bGLactive != TRUE) ActOpenGL();
			break;
		}

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
					pOrder->ui.notch_slew = 5; pOrder->ui.notch_slew_dir = 1;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO2) { 
					pOrder->ui.notch_slew = 0; pOrder->ui.notch_slew_dir = 0;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO3) {
					pOrder->ui.notch_slew = 5; pOrder->ui.notch_slew_dir = -1;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO4) {
					pOrder->ui.notch_bh = 5; pOrder->ui.notch_bh_dir = 1;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO5){
					pOrder->ui.notch_bh = 0; pOrder->ui.notch_bh_dir = 0;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO6) {
					pOrder->ui.notch_bh = 5; pOrder->ui.notch_bh_dir = -1;
				}
				else;
			}
			else if (inf.panel_func_id == IDC_TASK_FUNC_RADIO2) {
				wstring wstr, wstr_tmp;
				float tempf;
				int n;

				if (inf.panel_type_id == IDC_TASK_ITEM_RADIO1) {
					n = GetDlgItemText(hDlg, IDC_TASK_EDIT1, (LPTSTR)wstr_tmp.c_str(), 128);
					if (n)tempf = stof(wstr_tmp); else tempf = 0.0;
					pIO_Table->auto_ctrl.tgpos_slew += (double)tempf / COF_RAD2DEG;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO2) {
					n = GetDlgItemText(hDlg, IDC_TASK_EDIT1, (LPTSTR)wstr_tmp.c_str(), 128);
					if (n)tempf = stof(wstr_tmp); else tempf = 0.0;
					pIO_Table->auto_ctrl.tgpos_slew -= (double)tempf / COF_RAD2DEG;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO3) {
					pIO_Table->auto_ctrl.tgpos_slew = pIO_Table->physics.th;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO4) {
					n = GetDlgItemText(hDlg, IDC_TASK_EDIT2, (LPTSTR)wstr_tmp.c_str(), 128);
					if (n)tempf = stof(wstr_tmp); else tempf = 0.0;
					pIO_Table->auto_ctrl.tgpos_bh += (double)tempf;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO5) {
					n = GetDlgItemText(hDlg, IDC_TASK_EDIT2, (LPTSTR)wstr_tmp.c_str(), 128);
					if (n)tempf = stof(wstr_tmp); else tempf = 0.0;
					pIO_Table->auto_ctrl.tgpos_bh -= (double)tempf;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO6) {
					pIO_Table->auto_ctrl.tgpos_bh = pIO_Table->physics.R;
				}
				else;

				wstr = wstr + wstr_tmp.c_str();



			}
			else if (inf.panel_func_id == IDC_TASK_FUNC_RADIO4) {
				if (inf.panel_type_id == IDC_TASK_ITEM_RADIO1) VP_mode = 2;
			}
			else if (inf.panel_func_id == IDC_TASK_FUNC_RADIO6) {
				if (inf.panel_type_id == IDC_TASK_ITEM_RADIO1) pOrder->ui.env_mode = ENV_MODE_REAL;
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO2) pOrder->ui.env_mode = ENV_MODE_SIM1;
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO3) {
					if (pMode->antisway_hoist) pOrder->ui.as_mode_hoist = OPE_MODE_AS_OFF;
					else  pOrder->ui.as_mode_hoist = OPE_MODE_AS_ON;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO4) {
					if (pOrder->ui.ope_mode == OPE_MODE_MANUAL)pOrder->ui.ope_mode = OPE_MODE_AUTO_ENABLE;
					else pOrder->ui.ope_mode = OPE_MODE_MANUAL;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO5) {
					if(pOrder->ui.as_mode == OPE_MODE_AS_OFF)pOrder->ui.as_mode = OPE_MODE_AS_ON;
					else pOrder->ui.as_mode = OPE_MODE_AS_OFF;
				}
				else if (inf.panel_type_id == IDC_TASK_ITEM_RADIO6) {
					if (pOrder->ui.auto_mode != AUTO_MODE_ACTIVE)pOrder->ui.auto_mode = AUTO_MODE_ACTIVE;
					else pOrder->ui.auto_mode = AUTO_MODE_STANDBY;
				}
				else;
				pMan->get_UI();									//環境モード設定
			}
			else;
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

void CPublicRelation::set_panel_tip_txt()
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
		wstr = L"Type of Func2 \n\r 1:+slew tg 2:-slew tg 3:0 slew tg \n\r 4:+bh tg 5:-bh tg 6:0 bh tg";
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
		wstr = L"Type for Func4 \n\r 1:VP 2 2:?? 3:?? \n\r 4:?? 5:?? 6:??";
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
		wstr = L"Func6(Mode) \n\r 1:EnvReal 2:EnvSim1 3:AS_HOIST \n\r 4:AUTO 5:AS 6:AUTO_START";
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

void CPublicRelation::init_task(void *pobj) {
	pOrder->ui.env_mode = ENV_MODE_SIM1;
	pOrder->ui.as_mode = OPE_MODE_AS_OFF;
	pOrder->ui.auto_mode = AUTO_MODE_STANDBY;
	pOrder->ui.ope_mode = OPE_MODE_MANUAL;


	set_panel_tip_txt();
	return;
};


