#include "stdafx.h"
#include "CPlayer.h"

extern CMODE_Table*	pMode;				//共有メモリModeクラスポインタ
extern CORDER_Table*	pOrder;			//共有メモリOrderクラスポインタ
extern ST_SPEC			g_spec;			//クレーン仕様
extern CIO_Table*		pIO_Table;		//IO Table

CPlayer::CPlayer(){}

CPlayer::~CPlayer(){}

void CPlayer::routine_work(void *param) {

	if (pMode->environment == ENV_MODE_SIM1) cal_ui_order();
	else ;
			
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
