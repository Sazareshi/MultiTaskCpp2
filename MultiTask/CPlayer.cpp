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
		update_as_status();				//振止状態更新
		cal_as_ref();					//振止速度指令計算
	}

	set_table_out();				//出力セット
			
	ws << L" working!" << *(inf.psys_counter) % 100 << " SLEW_REF " << pIO_Table->ref.slew_w << " BH_REF " << pIO_Table->ref.bh_v;
	tweet2owner(ws.str()); ws.str(L""); ws.clear();

};

void CPlayer::cal_ui_order(){
	manual_vref[MOTION_ID_SLEW] = (double)(pOrder->ui.notch_slew_dir * g_spec.slew_notch_spd[pOrder->ui.notch_slew]);
	manual_vref[MOTION_ID_BH] = (double)(pOrder->ui.notch_bh_dir * g_spec.bh_notch_spd[pOrder->ui.notch_bh]);
	manual_vref[MOTION_ID_MH] = (double)(pOrder->ui.notch_mh_dir * g_spec.bh_notch_spd[pOrder->ui.notch_mh]);
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
	
	if (order_type == ORDER_TYPE_UI) {

	}

	if (order_type == ORDER_TYPE_MANU) {

	}

	return 0;
};
int CPlayer::update_as_status() {
	if (pMode->antisway == OFF) {
		bh_one_step[PLY_ID_AS_MOVE].status = STEP_FIN;
		slew_one_step[PLY_ID_AS_MOVE].status = STEP_FIN;
	}
	return 0;
};

int CPlayer::set_table_out(){
	
	if (pMode->environment == ENV_MODE_SIM1) {
		cal_ui_order();
	}
	else;

	if (pMode->antisway == OPE_MODE_AS_ON) {
		if(manual_vref[MOTION_ID_SLEW] != 0.0) 
			pIO_Table->ref.slew_w = manual_vref[MOTION_ID_SLEW];
		else 
			pIO_Table->ref.slew_w = as_vref[MOTION_ID_SLEW];

		if (manual_vref[MOTION_ID_BH] != 0.0) 
			pIO_Table->ref.bh_v = manual_vref[MOTION_ID_BH];
		else
			pIO_Table->ref.bh_v = as_vref[MOTION_ID_BH];

		if (manual_vref[MOTION_ID_MH] != 0.0)
			pIO_Table->ref.hoist_v = manual_vref[MOTION_ID_MH];
		else
			pIO_Table->ref.hoist_v = as_vref[MOTION_ID_MH];
	}
	else {
		pIO_Table->ref.slew_w = manual_vref[MOTION_ID_SLEW];
		pIO_Table->ref.bh_v = manual_vref[MOTION_ID_BH];
		pIO_Table->ref.hoist_v = manual_vref[MOTION_ID_MH];
	}

	return 0;
};

double CPlayer::act_one_step_slew(int mode, ST_MOTION_ELEMENT* recipe) {
	switch(mode){
	case PLY_ID_AUTO_MOVE:
	{
		; 
	}break;
	case PLY_ID_AS_MOVE:
	{
		;
	}break;
	default:break;

	}
	return 0.0;
};
double CPlayer::act_one_step_bh(int mode, ST_MOTION_ELEMENT* recipe) {
	switch (mode) {
	case PLY_ID_AUTO_MOVE:
	{
		;
	}break;
	case PLY_ID_AS_MOVE:
	{
		;
	}break;
	default:break;

	}
	return 0.0;
};
double CPlayer::act_one_step_mh(int mode, ST_MOTION_ELEMENT* recipe) {
	switch (mode) {
	case PLY_ID_AUTO_MOVE:
	{
		;
	}break;
	case PLY_ID_AS_MOVE:
	{
		;
	}break;
	default:break;

	}
	return 0.0;
};

//# Caliculate anti-sway reference
void CPlayer::cal_as_ref() {

	//## normal derection
	if (pMode->antisway_control_n & AS_MODE_ACTIVE_MOVE_LONG);
	else if (pMode->antisway_control_n & AS_MODE_ACTIVE_MOVE_SHORT);
	else if (pMode->antisway_control_n & AS_MODE_ACTIVE_INCH_SWAY);
	else if (pMode->antisway_control_n & AS_MODE_ACTIVE_INCH_POS);
	else;

	//## tangent derection
	if (pMode->antisway_control_t & AS_MODE_ACTIVE_MOVE_LONG);
	else if (pMode->antisway_control_t & AS_MODE_ACTIVE_MOVE_SHORT);
	else if (pMode->antisway_control_t & AS_MODE_ACTIVE_INCH_SWAY);
	else if (pMode->antisway_control_t & AS_MODE_ACTIVE_INCH_POS);
	else;

	//## vertical derection
	if (pMode->antisway_control_h & AS_MODE_ACTIVE_MOVE_LONG);
	else if (pMode->antisway_control_h & AS_MODE_ACTIVE_MOVE_SHORT);
	else if (pMode->antisway_control_h & AS_MODE_ACTIVE_INCH_SWAY);
	else if (pMode->antisway_control_h & AS_MODE_ACTIVE_INCH_POS);
	else;

	return ;
};

