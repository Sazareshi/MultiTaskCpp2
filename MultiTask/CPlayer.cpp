#include "stdafx.h"
#include "CPlayer.h"

extern CMODE_Table*	pMode;				//共有メモリModeクラスポインタ
extern CORDER_Table*	pOrder;			//共有メモリOrderクラスポインタ
extern ST_SPEC			g_spec;			//クレーン仕様
extern CIO_Table*		pIO_Table;		//IO Table

CPlayer::CPlayer(){}

CPlayer::~CPlayer(){}

void CPlayer::routine_work(void *param) {

	chk_as_status();			//振止状態更新
	
	if (pMode->environment == ENV_MODE_SIM1) {
		cal_ui_order();
	}
	else ;

	int set_table_out();			//出力セット
			
	ws << L" working!" << *(inf.psys_counter) % 100 << " SLEW_REF " << pIO_Table->ref.slew_w << " BH_REF " << pIO_Table->ref.bh_v;
	tweet2owner(ws.str()); ws.str(L""); ws.clear();

};

void CPlayer::cal_ui_order(){

	pIO_Table->ref.slew_w = (double)(pOrder->ui.notch_slew_dir * g_spec.slew_notch_spd[pOrder->ui.notch_slew]);
	pIO_Table->ref.bh_v = (double)(pOrder->ui.notch_bh_dir * g_spec.bh_notch_spd[pOrder->ui.notch_bh]);
	pIO_Table->ref.hoist_v = (double)(pOrder->ui.notch_mh_dir * g_spec.bh_notch_spd[pOrder->ui.notch_mh]);

};

void CPlayer::init_task(void *pobj) {
	set_panel_tip_txt();
	return;
};

int CPlayer::auto_start(LPST_JOB_ORDER recipe,int type) {
	if (type == AUTO_MODE_ACTIVE) {
		pMode->auto_control = AUTO_MODE_ACTIVE;
	}
	else pMode->auto_control = AUTO_MODE_STANDBY;

	wstring str = L"I got it";
	txout2msg_listbox(str);
	return 0;
};

/* ############################ */
/* PLAYERで管理するMODEの更新	*/
/* auto control mode			*/
/* antisway mode				*/
/* ############################ */

int CPlayer::update_mode(int order_type) {
	if (pMode->antisway != ON) {
		pMode->antisway_control = AS_MODE_DEACTIVATE;
	}
	else {
		switch (pMode->antisway_control) {
		case AS_MODE_ACTIVE:
			if(chk_as_status()) pMode->antisway_control = AS_MODE_STANDBY;
			break;
		case AS_MODE_DEACTIVATE:
			if (pMode->antisway == ON) {
				pMode->antisway_control = AS_MODE_STANDBY;
			}
			break;
		case AS_MODE_STANDBY:
			break;
		default:
			pMode->antisway_control = AS_MODE_DEACTIVATE;
			break;
		}
	}
	
	if (order_type == ORDER_TYPE_UI) {
		if ((pOrder->ui.anti_sway_trigger == ON)&&(pMode->antisway_control == AS_MODE_STANDBY)) {
			pMode->antisway_control = AS_MODE_ACTIVE;
		}
	}

	if (order_type == ORDER_TYPE_MANU) {

	}

	return 0;
};
int CPlayer::chk_as_status() {
	return 0;
};

int CPlayer::set_table_out(){
	return 0;
};

