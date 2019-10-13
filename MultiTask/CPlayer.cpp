#include "stdafx.h"
#include "CPlayer.h"
#include "CAnalyst.h"

extern CMODE_Table*	pMode;				//共有メモリModeクラスポインタ
extern CORDER_Table*pOrder;			//共有メモリOrderクラスポインタ
extern ST_SPEC		g_spec;			//クレーン仕様
extern CIO_Table*pIO_Table;		//IO Table

CPlayer::CPlayer(){}

CPlayer::~CPlayer(){}

void CPlayer::routine_work(void *param) {

	if (pMode->antisway == OPE_MODE_AS_ON) {
		update_as_status();			//振止状態更新
	}
	
	cal_auto_ref();					//PC計算速度指令計算

	set_table_out();				//出力セット
			
	ws << L" working!" << *(inf.psys_counter) % 100 << " SLEW_REF " << pIO_Table->ref.slew_w << " BH_REF " << pIO_Table->ref.bh_v<<"AS N POS16/SWAY8: "<< pMode->antisway_control_n;
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

int CPlayer::check_step_status_slew(LPST_MOTION_ELEMENT pStep) {
	int status = STEP_ON_GOING;
	switch (pStep->type) {
	case CTR_TYPE_TIME_WAIT:
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
		break;
	case CTR_TYPE_SINGLE_PHASE_WAIT: {
		double temp_chk1 = pStep->phase1 + pIO_Table->as_ctrl.phase_chk_range[AS_SLEW_ID];
		double temp_chk2 = pStep->phase1 - pIO_Table->as_ctrl.phase_chk_range[AS_SLEW_ID];

		if (temp_chk1 > DEF_PI) {
			if ((pIO_Table->physics.PhPlane_t.z < (temp_chk1 - DEF_2PI)) || (pIO_Table->physics.PhPlane_t.z > temp_chk2))
				status = STEP_FIN;
		}
		else if (temp_chk2 < -DEF_PI) {
			if ((pIO_Table->physics.PhPlane_t.z >(-temp_chk2 + DEF_2PI)) || (pIO_Table->physics.PhPlane_t.z < temp_chk1))
				status = STEP_FIN;
		}
		else {
			if ((pIO_Table->physics.PhPlane_t.z < temp_chk1) && (pIO_Table->physics.PhPlane_t.z > temp_chk2))
				status = STEP_FIN;
		}

		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;

	}break;

	case CTR_TYPE_DOUBLE_PHASE_WAIT: {
		double temp_chk1 = pStep->phase1 + pIO_Table->as_ctrl.phase_chk_range[AS_SLEW_ID];
		double temp_chk2 = pStep->phase1 - pIO_Table->as_ctrl.phase_chk_range[AS_SLEW_ID];

		if (temp_chk1 > DEF_PI) {
			if ((pIO_Table->physics.PhPlane_t.z < (temp_chk1 - DEF_2PI)) || (pIO_Table->physics.PhPlane_t.z > temp_chk2))
				status = STEP_FIN;
		}
		else if (temp_chk2 < -DEF_PI) {
			if ((pIO_Table->physics.PhPlane_t.z >(-temp_chk2 + DEF_2PI)) || (pIO_Table->physics.PhPlane_t.z < temp_chk1))
				status = STEP_FIN;
		}
		else {
			if ((pIO_Table->physics.PhPlane_n.z < temp_chk1) && (pIO_Table->physics.PhPlane_n.z > temp_chk2))
				status = STEP_FIN;
		}

		temp_chk1 = pStep->phase2 + pIO_Table->as_ctrl.phase_chk_range[AS_SLEW_ID];
		temp_chk2 = pStep->phase2 - pIO_Table->as_ctrl.phase_chk_range[AS_SLEW_ID];

		if (temp_chk1 > DEF_PI) {
			if ((pIO_Table->physics.PhPlane_t.z < (temp_chk1 - DEF_2PI)) || (pIO_Table->physics.PhPlane_t.z > temp_chk2))
				status = STEP_FIN;
		}
		else if (temp_chk2 < -DEF_PI) {
			if ((pIO_Table->physics.PhPlane_t.z >(-temp_chk2 + DEF_2PI)) || (pIO_Table->physics.PhPlane_t.z < temp_chk1))
				status = STEP_FIN;
		}
		else {
			if ((pIO_Table->physics.PhPlane_n.z < temp_chk1) && (pIO_Table->physics.PhPlane_n.z > temp_chk2))
				status = STEP_FIN;
		}

		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_ACC_AS_INCHING: {
	//	if (abs(pIO_Table->physics.wth) > abs(pStep->_v)) status = STEP_FIN;
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_DEC_V: {
		if (pStep->_v < 0.0) {
			if (pIO_Table->physics.wth >= pStep->_v) status = STEP_FIN;
		}
		else if (pStep->_v > 0.0) {
			if (pIO_Table->physics.wth <= pStep->_v) status = STEP_FIN;
		}
		else {
			if (abs(pIO_Table->physics.wth) < g_spec.bh_notch_spd[0] * 0.5) status = STEP_FIN;
		}
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_BH_WAIT:
	case CTR_TYPE_SLEW_WAIT:
	case CTR_TYPE_MH_WAIT:
	case CTR_TYPE_CONST_V_TIME:
	case CTR_TYPE_ACC_TIME:
	case CTR_TYPE_ACC_V:
	case CTR_TYPE_ACC_TIME_OR_V:
	case CTR_TYPE_DEC_TIME:
	case CTR_TYPE_DEC_TIME_OR_V:
	default:
		break;
	}
	return status;
}
int CPlayer::check_step_status_bh(LPST_MOTION_ELEMENT pStep) {
	int status = STEP_ON_GOING;

	switch (pStep->type) {
	case CTR_TYPE_TIME_WAIT:
		if(pStep->act_counter > pStep->time_count ) status = STEP_FIN;
		break;
	case CTR_TYPE_SINGLE_PHASE_WAIT: {
		double temp_chk1 = pStep->phase1 + pIO_Table->as_ctrl.phase_chk_range[AS_BH_ID];
		double temp_chk2 = pStep->phase1 - pIO_Table->as_ctrl.phase_chk_range[AS_BH_ID];

		if (temp_chk1 > DEF_PI) {
			if((pIO_Table->physics.PhPlane_n.z < (temp_chk1 - DEF_2PI )) ||	(pIO_Table->physics.PhPlane_n.z > temp_chk2))
				status = STEP_FIN;
		}
		else if (temp_chk2 < -DEF_PI) {
			if ((pIO_Table->physics.PhPlane_n.z > (-temp_chk2 + DEF_2PI)) || (pIO_Table->physics.PhPlane_n.z < temp_chk1))
				status = STEP_FIN;
		}
		else {
			if((pIO_Table->physics.PhPlane_n.z < temp_chk1)&&(pIO_Table->physics.PhPlane_n.z > temp_chk2))
				status = STEP_FIN;
		}

		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;

	}break;

	case CTR_TYPE_DOUBLE_PHASE_WAIT: {
		double temp_chk1 = pStep->phase1 + pIO_Table->as_ctrl.phase_chk_range[AS_BH_ID];
		double temp_chk2 = pStep->phase1 - pIO_Table->as_ctrl.phase_chk_range[AS_BH_ID];

		if (temp_chk1 > DEF_PI) {
			if ((pIO_Table->physics.PhPlane_n.z < (temp_chk1 - DEF_2PI)) || (pIO_Table->physics.PhPlane_n.z > temp_chk2))
				status = STEP_FIN;
		}
		else if (temp_chk2 < -DEF_PI) {
			if ((pIO_Table->physics.PhPlane_n.z >(-temp_chk2 + DEF_2PI)) || (pIO_Table->physics.PhPlane_n.z < temp_chk1))
				status = STEP_FIN;
		}
		else {
			if ((pIO_Table->physics.PhPlane_n.z < temp_chk1) && (pIO_Table->physics.PhPlane_n.z > temp_chk2))
				status = STEP_FIN;
		}

		temp_chk1 = pStep->phase2 + pIO_Table->as_ctrl.phase_chk_range[AS_BH_ID];
		temp_chk2 = pStep->phase2 - pIO_Table->as_ctrl.phase_chk_range[AS_BH_ID];

		if (temp_chk1 > DEF_PI) {
			if ((pIO_Table->physics.PhPlane_n.z < (temp_chk1 - DEF_2PI)) || (pIO_Table->physics.PhPlane_n.z > temp_chk2))
				status = STEP_FIN;
		}
		else if (temp_chk2 < -DEF_PI) {
			if ((pIO_Table->physics.PhPlane_n.z >(-temp_chk2 + DEF_2PI)) || (pIO_Table->physics.PhPlane_n.z < temp_chk1))
				status = STEP_FIN;
		}
		else {
			if ((pIO_Table->physics.PhPlane_n.z < temp_chk1) && (pIO_Table->physics.PhPlane_n.z > temp_chk2))
				status = STEP_FIN;
		}

		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_ACC_AS_INCHING: {
	//	if(abs(auto_vref[MOTION_ID_BH]) > abs(pStep->_v)) status = STEP_FIN;
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_DEC_V: {
		if(pStep->_v < 0.0){
			if(auto_vref[MOTION_ID_BH] >= pStep->_v) status = STEP_FIN;
		}
		else if (pStep->_v > 0.0){
			if (auto_vref[MOTION_ID_BH] <= pStep->_v) status = STEP_FIN;
		}
		else {
			if(abs(auto_vref[MOTION_ID_BH]) < g_spec.bh_notch_spd[0] * 0.5) status = STEP_FIN;
		}
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_BH_WAIT:
	case CTR_TYPE_SLEW_WAIT:
	case CTR_TYPE_MH_WAIT:
	case CTR_TYPE_CONST_V_TIME:
	case CTR_TYPE_ACC_TIME:
	case CTR_TYPE_ACC_V:
	case CTR_TYPE_ACC_TIME_OR_V:
	case CTR_TYPE_DEC_TIME:
	case CTR_TYPE_DEC_TIME_OR_V:
	default:
		break;
	}
	return status;
}
int CPlayer::check_step_status_mh(LPST_MOTION_ELEMENT pStep) {
	int status = STEP_FIN;
	switch (pStep->type) {
	default:
		break;
	}
	return status;
}

double CPlayer::act_slew_steps(ST_MOTION_UNIT* pRecipe) {
	double output_v = 0.0;
	LPST_MOTION_ELEMENT pStep = &pRecipe->motions[pRecipe->iAct];

	pStep->act_counter++;
	switch (pStep->type) {
	case CTR_TYPE_TIME_WAIT:
		output_v = pStep->_v;
		break;
	case CTR_TYPE_SINGLE_PHASE_WAIT: {
		output_v = pStep->_v;
	}break;

	case CTR_TYPE_DOUBLE_PHASE_WAIT: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_ACC_AS_INCHING: {
	//	if (pIO_Table->as_ctrl.as_out_dir_slew == 0) {
			if (abs(pIO_Table->physics.PhPlane_t.z) < DEF_HPI) pIO_Table->as_ctrl.as_out_dir_slew = +1;
			else  pIO_Table->as_ctrl.as_out_dir_slew = -1;
	//	}
		output_v = (double)pIO_Table->as_ctrl.as_out_dir_slew * pStep->_v;
		//output_v = (double)pIO_Table->as_ctrl.as_out_dir_slew * g_spec.slew_notch_spd[NOTCH_MAX - 1];
	}break;
	case CTR_TYPE_DEC_V: {
		output_v = pStep->_v;
		pIO_Table->as_ctrl.as_out_dir_slew = 0;
	}break;
	case CTR_TYPE_BH_WAIT:
	case CTR_TYPE_SLEW_WAIT:
	case CTR_TYPE_MH_WAIT:
	case CTR_TYPE_CONST_V_TIME:
	case CTR_TYPE_ACC_TIME:
	case CTR_TYPE_ACC_V:
	case CTR_TYPE_ACC_TIME_OR_V:
	case CTR_TYPE_DEC_TIME:
	case CTR_TYPE_DEC_TIME_OR_V:
	default:
		break;
	}
	if (STEP_FIN == check_step_status_slew(pStep)) pRecipe->iAct++;
	return output_v;
};
double CPlayer::act_bh_steps(ST_MOTION_UNIT* pRecipe) {
	double output_v = 0.0;
	LPST_MOTION_ELEMENT pStep = &pRecipe->motions[pRecipe->iAct];

	pStep->act_counter++;
	switch (pStep->type) {
	case CTR_TYPE_TIME_WAIT:
		output_v = pStep->_v;
		break;
	case CTR_TYPE_SINGLE_PHASE_WAIT: {
		output_v = pStep->_v;
	}break;

	case CTR_TYPE_DOUBLE_PHASE_WAIT: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_ACC_AS_INCHING: {
		if (abs(pIO_Table->physics.PhPlane_n.z) < DEF_HPI) pIO_Table->as_ctrl.as_out_dir_bh = 1;
		else  pIO_Table->as_ctrl.as_out_dir_bh = -1;
		output_v = (double)pIO_Table->as_ctrl.as_out_dir_bh * pStep->_v;
	//	output_v = (double)pIO_Table->as_ctrl.as_out_dir_bh * g_spec.bh_notch_spd[NOTCH_MAX - 1];

	//	if(abs(pIO_Table->physics.PhPlane_n.z) < DEF_HPI)  output_v = g_spec.bh_notch_spd[NOTCH_MAX-1];
	//	else   output_v = -g_spec.bh_notch_spd[NOTCH_MAX - 1];
	}break;
	case CTR_TYPE_DEC_V: {
		output_v = pStep->_v;
		pIO_Table->as_ctrl.as_out_dir_bh = 0;
	}break;
	case CTR_TYPE_BH_WAIT:
	case CTR_TYPE_SLEW_WAIT:
	case CTR_TYPE_MH_WAIT:
	case CTR_TYPE_CONST_V_TIME:
	case CTR_TYPE_ACC_TIME:
	case CTR_TYPE_ACC_V:
	case CTR_TYPE_ACC_TIME_OR_V:
	case CTR_TYPE_DEC_TIME:
	case CTR_TYPE_DEC_TIME_OR_V:
	default:
		break;
	}
	if (STEP_FIN == check_step_status_bh(pStep)) pRecipe->iAct++;
	return output_v;
};
double CPlayer::act_mh_steps(ST_MOTION_UNIT* pRecipe) {
	double output_v = 0.0;


	return output_v;
};

//# Caliculate PC reference
void CPlayer::cal_auto_ref() {
	
	CAnalyst* pAna = (CAnalyst*)VectpCTaskObj[g_itask.ana];

	if (pMode->auto_control == AUTO_MODE_ACTIVE) {
		;
	}
	else if (pMode->antisway == OPE_MODE_AS_ON){
		update_as_status(); //Check Current Pattern Handling Situation
		//## normal derection
		if ((bh_motion_ptn.ptn_status == PTN_UNIT_FIN) || (bh_motion_ptn.ptn_status == PTN_NOTHING)) {//Any pattern not running
			auto_vref[MOTION_ID_BH] = 0.0;
			if (pMode->antisway_control_n != AS_MODE_DEACTIVATE) {
				//Set pattern recipe
				if (pAna->cal_as_inch_recipe(MOTION_ID_BH, &(this->bh_motion_ptn)) == NO_ERR_EXIST) {
					bh_motion_ptn.ptn_status = PTN_STANDBY;
				}
			}
		}
		else if (bh_motion_ptn.ptn_status == PTN_PAUSE){
			auto_vref[MOTION_ID_BH] = 0.0;
		}
		else if (bh_motion_ptn.ptn_status == PTN_STANDBY) {
			bh_motion_ptn.ptn_status = PTN_ACTIVE;
		}
		else if (bh_motion_ptn.ptn_status == PTN_ACTIVE) {
			if (bh_motion_ptn.iAct > bh_motion_ptn.n_step) {
				bh_motion_ptn.ptn_status = PTN_UNIT_FIN;
				auto_vref[MOTION_ID_BH] = 0.0;
			}
			else if (bh_motion_ptn.type != BH_AXIS) {
				bh_motion_ptn.ptn_status = PTN_NOTHING;
				auto_vref[MOTION_ID_BH] = 0.0;
			}
			else{
				auto_vref[MOTION_ID_BH] = act_bh_steps(&(this->bh_motion_ptn));
			}
		}
		else {
			auto_vref[MOTION_ID_BH] = 0.0;
		}
		//## tangent derection
		if ((slew_motion_ptn.ptn_status == PTN_UNIT_FIN) || (slew_motion_ptn.ptn_status == PTN_NOTHING)) {//Any pattern not running
			auto_vref[MOTION_ID_SLEW] = 0.0;
			if (pMode->antisway_control_t != AS_MODE_DEACTIVATE) {
				//Set pattern recipe
				if (pAna->cal_as_inch_recipe(MOTION_ID_SLEW, &(this->slew_motion_ptn)) == NO_ERR_EXIST) {
					slew_motion_ptn.ptn_status = PTN_STANDBY;
				}
			}
		}
		else if (slew_motion_ptn.ptn_status == PTN_PAUSE) {
			auto_vref[MOTION_ID_SLEW] = 0.0;
		}
		else if (slew_motion_ptn.ptn_status == PTN_STANDBY) {
			slew_motion_ptn.ptn_status = PTN_ACTIVE;
		}
		else if (slew_motion_ptn.ptn_status == PTN_ACTIVE) {
			if (slew_motion_ptn.iAct > slew_motion_ptn.n_step) {
				slew_motion_ptn.ptn_status = PTN_UNIT_FIN;
				auto_vref[MOTION_ID_SLEW] = 0.0;
			}
			else if (slew_motion_ptn.type != SLW_AXIS) {
				slew_motion_ptn.ptn_status = PTN_NOTHING;
				auto_vref[MOTION_ID_SLEW] = 0.0;
			}
			else {
				auto_vref[MOTION_ID_SLEW] = act_slew_steps(&(this->slew_motion_ptn));
			}
		}
		else {
			auto_vref[MOTION_ID_SLEW] = 0.0;
		}
	}
	else {
		auto_vref[MOTION_ID_SLEW] = 0.0;
		auto_vref[MOTION_ID_MH] = 0.0;
		auto_vref[MOTION_ID_BH] = 0.0;
	}
	
	return ;
};

