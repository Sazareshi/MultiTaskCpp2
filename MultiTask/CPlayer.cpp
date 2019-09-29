#include "stdafx.h"
#include "CPlayer.h"

extern CMODE_Table*	pMode;				//共有メモリModeクラスポインタ
extern CORDER_Table*	pOrder;			//共有メモリOrderクラスポインタ
extern ST_SPEC			g_spec;			//クレーン仕様
extern CIO_Table*		pIO_Table;		//IO Table

CPlayer::CPlayer(){}

CPlayer::~CPlayer(){}

void CPlayer::routine_work(void *param) {

	if (pMode->antisway == OPE_MODE_AS_ON) {
		update_as_status();			//振止状態更新
	}
	
	cal_auto_ref();					//PC計算速度指令計算

	set_table_out();				//出力セット
			
	ws << L" working!" << *(inf.psys_counter) % 100 << " SLEW_REF " << pIO_Table->ref.slew_w << " BH_REF " << pIO_Table->ref.bh_v;
tweet2owner(ws.str()); ws.str(L""); ws.clear();

};

void CPlayer::cal_ui_order() {
	manual_vref[MOTION_ID_SLEW] = (double)(pOrder->ui.notch_slew_dir * g_spec.slew_notch_spd[pOrder->ui.notch_slew]);
	manual_vref[MOTION_ID_BH] = (double)(pOrder->ui.notch_bh_dir * g_spec.bh_notch_spd[pOrder->ui.notch_bh]);
	manual_vref[MOTION_ID_MH] = (double)(pOrder->ui.notch_mh_dir * g_spec.bh_notch_spd[pOrder->ui.notch_mh]);
};

void CPlayer::init_task(void *pobj) {
	set_panel_tip_txt();
	return;
};

int CPlayer::auto_start(LPST_JOB_ORDER recipe, int type) {
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

	if (order_type == ORDER_TYPE_UI) {

	}

	if (order_type == ORDER_TYPE_MANU) {

	}

	return 0;
};
int CPlayer::update_as_status() {
	if (pMode->antisway == OFF) {
		bh_motion_ptn.ptn_status = PTN_UNIT_FIN;
		slew_motion_ptn.ptn_status = PTN_UNIT_FIN;
	}
	return 0;
};

int CPlayer::set_table_out() {

	if (pMode->environment == ENV_MODE_SIM1) {
		cal_ui_order();
	}
	else;

	if ((pMode->antisway == OPE_MODE_AS_ON) || (pMode->auto_control == AUTO_MODE_ACTIVE)) {
		if (manual_vref[MOTION_ID_SLEW] != 0.0)
			pIO_Table->ref.slew_w = manual_vref[MOTION_ID_SLEW];
		else
			pIO_Table->ref.slew_w = auto_vref[MOTION_ID_SLEW];

		if (manual_vref[MOTION_ID_BH] != 0.0)
			pIO_Table->ref.bh_v = manual_vref[MOTION_ID_BH];
		else
			pIO_Table->ref.bh_v = auto_vref[MOTION_ID_BH];

		if (manual_vref[MOTION_ID_MH] != 0.0)
			pIO_Table->ref.hoist_v = manual_vref[MOTION_ID_MH];
		else
			pIO_Table->ref.hoist_v = auto_vref[MOTION_ID_MH];
	}
	else {
		pIO_Table->ref.slew_w = manual_vref[MOTION_ID_SLEW];
		pIO_Table->ref.bh_v = manual_vref[MOTION_ID_BH];
		pIO_Table->ref.hoist_v = manual_vref[MOTION_ID_MH];
	}


	if(manual_vref[MOTION_ID_BH] != 0.0) 
		pIO_Table->ref.b_bh_manual_ctrl= TRUE;		//手動操作中
	else if(pIO_Table->physics.vR == 0.0)
		pIO_Table->ref.b_bh_manual_ctrl = FALSE;	//手動操作中解除

	if (manual_vref[MOTION_ID_SLEW] != 0.0)
		pIO_Table->ref.b_slew_manual_ctrl = TRUE;	//手動操作中
	else if (pIO_Table->physics.wth == 0.0)
		pIO_Table->ref.b_slew_manual_ctrl = FALSE;	//手動操作中解除

	if (manual_vref[MOTION_ID_MH] != 0.0)
		pIO_Table->ref.b_mh_manual_ctrl = TRUE;		//手動操作中
	else if (pIO_Table->physics.vL == 0.0)
		pIO_Table->ref.b_mh_manual_ctrl = FALSE;	//手動操作中解除

	return 0;
};

double CPlayer::act_slew_steps(int mode, ST_MOTION_UNIT* recipe) {
	return 0.0;
};
double CPlayer::act_bh_steps(int mode, ST_MOTION_UNIT* recipe) {
	return 0.0;
};
double CPlayer::act_mh_steps(int mode, ST_MOTION_UNIT* recipe) {
	return 0.0;
};

//# Caliculate PC reference
void CPlayer::cal_auto_ref() {

	if (pMode->auto_control == AUTO_MODE_ACTIVE) {
		;
	}
	else if (pMode->antisway == OPE_MODE_AS_ON){
		//## normal derection
		if ((bh_motion_ptn.ptn_status == PTN_UNIT_FIN) || (bh_motion_ptn.ptn_status == PTN_NOTHING)) {
			auto_vref[MOTION_ID_BH] = 0.0;
			if (pMode->antisway_control_n != AS_MODE_DEACTIVATE) {

			}
		}

		//## tangent derection


		//## vertical derection

	}
	else {
		auto_vref[MOTION_ID_SLEW] = 0.0;
		auto_vref[MOTION_ID_MH] = 0.0;
		auto_vref[MOTION_ID_BH] = 0.0;
	}
	
	return ;
};

