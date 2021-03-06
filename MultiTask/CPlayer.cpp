#include "stdafx.h"
#include "CPlayer.h"
#include "CAnalyst.h"
#include "CManager.h"

extern CMODE_Table*	pMode;				//共有メモリModeクラスポインタ
extern CORDER_Table*pOrder;			//共有メモリOrderクラスポインタ
extern ST_SPEC		g_spec;			//クレーン仕様
extern CIO_Table*pIO_Table;		//IO Table

CPlayer::CPlayer(){
	p_motion_ptn[BH_AXIS] = &motion_ptn[BH_AXIS];
	p_motion_ptn[SLW_AXIS] = &motion_ptn[SLW_AXIS];
	p_motion_ptn[MH_AXIS] = &motion_ptn[MH_AXIS];
}

CPlayer::~CPlayer(){}

void CPlayer::routine_work(void *param) {

	update_auto_status();			//自動振止状態更新
	cal_auto_ref();					//PC計算 速度指令計算
	set_table_out();				//出力セット
			
	//ws << L" working!" << *(inf.psys_counter) % 100 << " BH STAT PTN " <<p_motion_ptn[BH_AXIS]->ptn_status << " BH STEP " << p_motion_ptn[BH_AXIS]->iAct <<"AS N POS16/SWAY8: "<< pMode->antisway_control_n;

	//ws << "PHASE r:" << pIO_Table->physics.PhPlane_r.z << "PHASE rw:" << pIO_Table->physics.wPhPlane_r << "   PHASE amp: "<< pIO_Table->physics.sway_amp_r_ph << " MotionSLEW:" << p_motion_ptn[SLW_AXIS]->motion_type<<"    MotionBH: " << p_motion_ptn[BH_AXIS]->motion_type;
	ws<< "AS_STP_T:" << p_motion_ptn[SLW_AXIS]->motions[p_motion_ptn[SLW_AXIS]->iAct].type << " AS_STP_N: " << p_motion_ptn[BH_AXIS]->motions[p_motion_ptn[BH_AXIS]->iAct].type << "AS_PTN_T:" << p_motion_ptn[SLW_AXIS]->motion_type<<" AS_PTN_N: " << p_motion_ptn[BH_AXIS]->motion_type << " PH n:" << pIO_Table->physics.PhPlane_n.z << " PH t:"  << pIO_Table->physics.PhPlane_t.z ;

	tweet2owner(ws.str()); ws.str(L""); ws.clear(); 

};

//### 操作卓指令計算
void CPlayer::cal_console_order() {
	if(pIO_Table->console_remote.slew_notch_dir)
		manual_vref[MOTION_ID_SLEW] = (double)(pIO_Table->console_remote.slew_notch_dir * g_spec.slew_notch_spd[pIO_Table->console_remote.slew_notch]);
	else if(pOrder->ui.notch_slew_dir)
		manual_vref[MOTION_ID_SLEW] = (double)(pOrder->ui.notch_slew_dir * g_spec.slew_notch_spd[pOrder->ui.notch_slew]);
	else manual_vref[MOTION_ID_SLEW] = 0.0;

	if (pIO_Table->console_remote.bh_notch_dir)
		manual_vref[MOTION_ID_BH] = (double)(pIO_Table->console_remote.bh_notch_dir * g_spec.bh_notch_spd[pIO_Table->console_remote.bh_notch]);
	else if (pOrder->ui.notch_bh_dir)
		manual_vref[MOTION_ID_BH] = (double)(pOrder->ui.notch_bh_dir * g_spec.bh_notch_spd[pOrder->ui.notch_bh]);
	else manual_vref[MOTION_ID_BH] =0.0;

	if (pIO_Table->console_remote.mh_notch_dir)
		manual_vref[MOTION_ID_MH] = (double)(pIO_Table->console_remote.mh_notch_dir * g_spec.hoist_notch_spd[pIO_Table->console_remote.mh_notch]);
	else if (pOrder->ui.notch_mh_dir)
		manual_vref[MOTION_ID_MH] = (double)(pOrder->ui.notch_mh_dir * g_spec.hoist_notch_spd[pOrder->ui.notch_mh]);
	else manual_vref[MOTION_ID_MH] = 0.0;
};
//### タスク初期化
void CPlayer::init_task(void *pobj) {
	set_panel_tip_txt();
	return;
};

/* ############################ */
/* PLAYERで管理するMODEの更新	*/
/* auto control mode			*/
/* antisway mode				*/
/* ############################ */

int CPlayer::update_auto_status() {
	if (pMode->antisway == OFF) {
		motion_ptn[BH_AXIS].ptn_status = PTN_UNIT_FIN;
		motion_ptn[SLW_AXIS].ptn_status = PTN_UNIT_FIN;
		motion_ptn[MH_AXIS].ptn_status = PTN_UNIT_FIN;
	}
	return 0;
};
int CPlayer::set_table_out() {

	if (pMode->environment == ENV_MODE_SIM1) {
		cal_console_order();
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
	else;

	if (manual_vref[MOTION_ID_SLEW] != 0.0)
		pIO_Table->ref.b_slew_manual_ctrl = TRUE;	//手動操作中
	else if (pIO_Table->physics.wth == 0.0)
		pIO_Table->ref.b_slew_manual_ctrl = FALSE;	//手動操作中解除
	else;

	if (manual_vref[MOTION_ID_MH] != 0.0)
		pIO_Table->ref.b_mh_manual_ctrl = TRUE;		//手動操作中
	else if (pIO_Table->physics.vL == 0.0)
		pIO_Table->ref.b_mh_manual_ctrl = FALSE;	//手動操作中解除
	else;

	return 0;
};
int CPlayer::set_motion_receipe() {

	CAnalyst* pAna = (CAnalyst*)VectpCTaskObj[g_itask.ana];
	CManager* pMan = (CManager*)VectpCTaskObj[g_itask.mng];

	//### normal direction
	if((pMode->antisway == OPE_MODE_AS_ON) //自動可モードでパターンの準備完了
		&&(pMode->operation & OPE_MODE_AUTO_ENABLE) && (pOrder->job_A.status == JOB_ORDER_STEP_STANDBY)
		&& ((p_motion_ptn[BH_AXIS]->ptn_status == PTN_UNIT_FIN) || (p_motion_ptn[BH_AXIS]->ptn_status == PTN_NOTHING))
		&& ((p_motion_ptn[SLW_AXIS]->ptn_status == PTN_UNIT_FIN) || (p_motion_ptn[SLW_AXIS]->ptn_status == PTN_NOTHING)))
	{
#if 0
		p_motion_ptn[SLW_AXIS] = &(pOrder->job_A.command_recipe[pOrder->job_A.job_step_now].motions[SLW_AXIS]);//レシピ構造体セット
		p_motion_ptn[BH_AXIS] = &(pOrder->job_A.command_recipe[pOrder->job_A.job_step_now].motions[BH_AXIS]);//レシピ構造体セット
		pAna->cal_auto_target(CAL_FOR_AUTO_JOB_A);

		//Set pattern recipe
		//Judge Positioning or Damping
		double check_d = pIO_Table->physics.T * g_spec.bh_notch_spd[1] //1ノッチ一周期　＋　振れ止め1回の移動距離
			+ g_spec.bh_acc[FWD_ACC] * pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID] * pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID];
		double check_d2 = pIO_Table->physics.T * g_spec.bh_notch_spd[1] / 3.0 //1ノッチ 1/3周期　＋　1ノッチインチング距離
			+ g_spec.bh_notch_spd[1] * g_spec.bh_notch_spd[1] / g_spec.bh_acc[FWD_ACC];

		if (pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] > check_d) {
			if (pAna->cal_move_trapezoid(MOTION_ID_BH, p_motion_ptn[BH_AXIS], AUTO_PTN_MODE_AUTOMOVE) == NO_ERR_EXIST) {
				p_motion_ptn[BH_AXIS]->ptn_status = PTN_STANDBY;
			}
		}
		else if ((pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] > check_d2) && (pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] > 0.5)) {
			if (pAna->cal_move_3Step(MOTION_ID_BH, p_motion_ptn[BH_AXIS], AUTO_PTN_MODE_AUTOMOVE) == NO_ERR_EXIST) {
				p_motion_ptn[BH_AXIS]->ptn_status = PTN_STANDBY;
			}
		}
		else {
			if (pMode->antisway_control_n == AS_MOVE_ANTISWAY) {
				if (pAna->cal_move_1Step(MOTION_ID_BH, p_motion_ptn[BH_AXIS], AUTO_PTN_MODE_AUTOMOVE) == NO_ERR_EXIST) {
					p_motion_ptn[BH_AXIS]->ptn_status = PTN_STANDBY;
				}
			}
		}

		if (p_motion_ptn[SLW_AXIS]->ptn_status != PTN_STANDBY) {
			p_motion_ptn[BH_AXIS]->ptn_status = PTN_UNIT_FIN;
		}


		pMan->handle_order_event(ORDER_ID_JOB_A, JOB_EVENT_ACTIVATE_COMMAND_STEP, pOrder->job_A.job_step_now);
#endif
	}
	else if (pMode->antisway == OPE_MODE_AS_ON) { //手動振れ止め時

		if ((p_motion_ptn[BH_AXIS]->ptn_status == PTN_UNIT_FIN) || (p_motion_ptn[BH_AXIS]->ptn_status == PTN_NOTHING)) {//Any pattern not running

			p_motion_ptn[BH_AXIS] = &motion_ptn[BH_AXIS];//レシピ構造体セット

			if (pMode->antisway_control_n != AS_MOVE_DEACTIVATE) {//Set pattern recipe

				//振れ止め完了
				if (pMode->antisway_control_n == AS_MOVE_COMPLETE) {
					p_motion_ptn[BH_AXIS]->n_step = 0;
				}
				else if (pMode->antisway_ptn_n == AS_PTN_1STEP) {
					if (pAna->cal_move_1Step(MOTION_ID_BH, p_motion_ptn[BH_AXIS], AUTO_PTN_MODE_SINGLE) == NO_ERR_EXIST)
							p_motion_ptn[BH_AXIS]->ptn_status = PTN_STANDBY;
				}
				else if (pMode->antisway_ptn_n == AS_PTN_2ACCDEC) { 
					if(pAna->cal_move_2accdec(MOTION_ID_BH, p_motion_ptn[BH_AXIS], AUTO_PTN_MODE_SINGLE))
					        p_motion_ptn[BH_AXIS]->ptn_status = PTN_STANDBY;
				}
				else if (pMode->antisway_ptn_n == AS_PTN_2STEP_PN) {
					if (pAna->cal_move_2Step_pn(MOTION_ID_BH, p_motion_ptn[BH_AXIS], AUTO_PTN_MODE_SINGLE) == NO_ERR_EXIST)
						p_motion_ptn[BH_AXIS]->ptn_status = PTN_STANDBY;
				}
				else if (pMode->antisway_ptn_n == AS_PTN_2STEP_PP){
					if (pAna->cal_move_2Step_pp(MOTION_ID_BH, p_motion_ptn[BH_AXIS], AUTO_PTN_MODE_SINGLE) == NO_ERR_EXIST) {
						p_motion_ptn[BH_AXIS]->ptn_status = PTN_STANDBY;
					}
				}
				else if (pMode->antisway_ptn_n == AS_PTN_3STEP) {
					if (pAna->cal_move_3Step(MOTION_ID_BH, p_motion_ptn[BH_AXIS], AUTO_PTN_MODE_SINGLE) == NO_ERR_EXIST) {
						p_motion_ptn[BH_AXIS]->ptn_status = PTN_STANDBY;
					}
				}
				else if (pMode->antisway_ptn_n == AS_PTN_2ACCDEC) {
					if (pAna->cal_move_2accdec(MOTION_ID_BH, p_motion_ptn[BH_AXIS], AUTO_PTN_MODE_SINGLE))
						p_motion_ptn[BH_AXIS]->ptn_status = PTN_STANDBY;
				}
				else {
					p_motion_ptn[BH_AXIS]->ptn_status = PTN_NOTHING;
				}
			}
		}
		//### tangent direction
		if ((p_motion_ptn[SLW_AXIS]->ptn_status == PTN_UNIT_FIN) || (p_motion_ptn[SLW_AXIS]->ptn_status == PTN_NOTHING)) {//Any pattern not running

			p_motion_ptn[SLW_AXIS] = &motion_ptn[SLW_AXIS];//レシピ構造体セット

			if (pMode->antisway_control_t != AS_MOVE_DEACTIVATE) {
				if (pMode->antisway_control_t == AS_MOVE_COMPLETE) {
					p_motion_ptn[SLW_AXIS]->n_step = 0;
				}
				else if (pMode->antisway_ptn_t == AS_PTN_1STEP) {
					if (pAna->cal_move_1Step(MOTION_ID_SLEW, p_motion_ptn[SLW_AXIS], AUTO_PTN_MODE_SINGLE) == NO_ERR_EXIST)
						p_motion_ptn[SLW_AXIS]->ptn_status = PTN_STANDBY;
				}
				else if (pMode->antisway_ptn_t == AS_PTN_2ACCDEC) {
					if (pAna->cal_move_2accdec(MOTION_ID_SLEW, p_motion_ptn[SLW_AXIS], AUTO_PTN_MODE_SINGLE))
						p_motion_ptn[SLW_AXIS]->ptn_status = PTN_STANDBY;
				}
				else if (pMode->antisway_ptn_t== AS_PTN_2STEP_PN) {
					if (pAna->cal_move_2Step_pn(MOTION_ID_SLEW, p_motion_ptn[SLW_AXIS], AUTO_PTN_MODE_SINGLE) == NO_ERR_EXIST) {
						p_motion_ptn[SLW_AXIS]->ptn_status = PTN_STANDBY;
					}
				}
				else if (pMode->antisway_ptn_t == AS_PTN_2STEP_PP) {
					if (pAna->cal_move_2Step_pp(MOTION_ID_SLEW, p_motion_ptn[SLW_AXIS], AUTO_PTN_MODE_SINGLE) == NO_ERR_EXIST) {
						p_motion_ptn[SLW_AXIS]->ptn_status = PTN_STANDBY;
					}
				}
				else if (pMode->antisway_ptn_t == AS_PTN_3STEP) {
					if (pAna->cal_move_3Step(MOTION_ID_SLEW, p_motion_ptn[SLW_AXIS], AUTO_PTN_MODE_SINGLE) == NO_ERR_EXIST) {
						p_motion_ptn[SLW_AXIS]->ptn_status = PTN_STANDBY;
					}
				}
				else if (pMode->antisway_ptn_t == AS_PTN_2ACCDEC) {
					if (pAna->cal_move_2accdec(MOTION_ID_SLEW, p_motion_ptn[SLW_AXIS], AUTO_PTN_MODE_SINGLE))
						p_motion_ptn[SLW_AXIS]->ptn_status = PTN_STANDBY;
				}
				else {
					p_motion_ptn[SLW_AXIS]->ptn_status = PTN_NOTHING;
				}
			}
		}
		//### hoist direction
		if ((p_motion_ptn[MH_AXIS]->ptn_status == PTN_UNIT_FIN) || (p_motion_ptn[MH_AXIS]->ptn_status == PTN_NOTHING)) {//Any pattern not running

			p_motion_ptn[MH_AXIS] = &motion_ptn[MH_AXIS];//レシピ構造体セット

			if (pMode->antisway_control_h==AS_MOVE_ANTISWAY){
				//Set pattern recipe
				if (pAna->cal_move_2Step_pn(MOTION_ID_MH, p_motion_ptn[MH_AXIS], AUTO_PTN_MODE_SINGLE) == NO_ERR_EXIST) {
					p_motion_ptn[MH_AXIS]->ptn_status = PTN_STANDBY;
				}
			}
		}
	}
	else;
		
	return 0;
};
int CPlayer::check_step_status_slew(LPST_MOTION_ELEMENT pStep) {
	int status = STEP_ON_GOING;
	switch (pStep->type) {
	case CTR_TYPE_TIME_WAIT: {
		if (pStep->act_counter > pStep->time_count) {
			status = STEP_FIN;
		}
	}break;
	case CTR_TYPE_TIME_WAIT_2PN: {
		if (pStep->opt_i2 == AS_INIT_SWAY_SMALL) {
			if (pStep->act_counter > (pStep->time_count - pStep->opt_i1)) {//初期振れ分停止時間短縮
				status = STEP_FIN;
				if (pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] == AS_DIR_PLUS) {
					pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_MINUS;//移動方向切替
				}
				else {
					pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_PLUS;
				}
			}
		}
		else {

			if (pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] == AS_DIR_PLUS) {
				if (pIO_Table->auto_ctrl.as_start_ph[AS_SLEW_ID] == AS_START_LOW_PHASE) {
					if (pStep->act_counter > (pStep->time_count + pStep->opt_i1)) {//初期振れ分停止時間延長
						status = STEP_FIN;
						pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_MINUS;//移動方向切替
					}
				}
				else if (pIO_Table->auto_ctrl.as_start_ph[AS_SLEW_ID] == AS_START_HIGH_PHASE) {
					if (pStep->act_counter > (pStep->time_count - pStep->opt_i1)) {//初期振れ分停止時間短縮
						status = STEP_FIN;
						pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_MINUS;//移動方向切替
					}
				}
				else {
					if (pStep->act_counter > pStep->time_count) {
						status = STEP_FIN;
						pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_MINUS;//移動方向切替
					}
				}
			}
			else {
				if (pIO_Table->auto_ctrl.as_start_ph[AS_SLEW_ID] == AS_START_LOW_PHASE) {
					if (pStep->act_counter > (pStep->time_count - pStep->opt_i1)) {//初期振れ分停止時間短縮
						status = STEP_FIN;
						pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_PLUS;//移動方向切替
					}
				}
				else if (pIO_Table->auto_ctrl.as_start_ph[AS_SLEW_ID] == AS_START_HIGH_PHASE) {
					if (pStep->act_counter > (pStep->time_count + pStep->opt_i1)) {//初期振れ分停止時間延長
						status = STEP_FIN;
						pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_PLUS;//移動方向切替
					}
				}
				else {
					if (pStep->act_counter > pStep->time_count) {//初期振れ分停止時間短縮
						status = STEP_FIN;
						pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_PLUS;//移動方向切替
					}
				}
			}
		}
	}break;
	case CTR_TYPE_TIME_WAIT_2PP: {
		if (pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] == AS_DIR_PLUS) {
			if (pIO_Table->auto_ctrl.as_start_ph[AS_SLEW_ID] == AS_START_HIGH_PHASE) {
				if (pStep->act_counter > (pStep->time_count - pStep->opt_i1)) {//初期振れ分停止時間短縮
					status = STEP_FIN;
				}
			}
			else {
				if (pStep->act_counter > (pStep->time_count + pStep->opt_i1)) {//初期振れ分停止時間延長
					status = STEP_FIN;
				}
			}
		}
		else {
			if (pIO_Table->auto_ctrl.as_start_ph[AS_SLEW_ID] == AS_START_HIGH_PHASE) {
				if (pStep->act_counter > (pStep->time_count + pStep->opt_i1)) {//初期振れ分停止時間延長
					status = STEP_FIN;
				}
			}
			else {
				if (pStep->act_counter > (pStep->time_count - pStep->opt_i1)) {//初期振れ分停止時間短縮
					status = STEP_FIN;
				}
			}
		}
	}break;
	case CTR_TYPE_SINGLE_PHASE_WAIT: {
		double temp_chk1 = pStep->phase1 + pIO_Table->auto_ctrl.phase_chk_range[AS_SLEW_ID];
		double temp_chk2 = pStep->phase1 - pIO_Table->auto_ctrl.phase_chk_range[AS_SLEW_ID];

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

		//Too small sway
		if (pIO_Table->physics.sway_amp_t_ph < g_spec.as_compl_swayLv[I_AS_LV_COMPLE]) {
			status = STEP_FIN;
		}

		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;

	}break;
	case CTR_TYPE_DOUBLE_PHASE_WAIT: {

		//Low phase
		double temp_chk1 = pStep->phase1 + pIO_Table->auto_ctrl.phase_chk_range[AS_SLEW_ID];
		double temp_chk2 = pStep->phase1 - pIO_Table->auto_ctrl.phase_chk_range[AS_SLEW_ID];

		if (abs(pStep->phase1) > DEF_PI) {
			status = STEP_ERROR;
		}
		else if (temp_chk1 > DEF_PI) {
			if ((pIO_Table->physics.PhPlane_t.z < (temp_chk1 - DEF_2PI)) || (pIO_Table->physics.PhPlane_t.z > temp_chk2))
				status = STEP_FIN;
		}
		else if (temp_chk2 < -DEF_PI) {
			if ((pIO_Table->physics.PhPlane_t.z > (-temp_chk2 + DEF_2PI)) || (pIO_Table->physics.PhPlane_t.z < temp_chk1))
				status = STEP_FIN;
		}
		else {
			if ((pIO_Table->physics.PhPlane_t.z < temp_chk1) && (pIO_Table->physics.PhPlane_t.z > temp_chk2))
				status = STEP_FIN;
		}

		if ((status == STEP_FIN)||(status == STEP_ERROR)) {
			pIO_Table->auto_ctrl.as_start_ph[AS_SLEW_ID] = AS_START_LOW_PHASE;
			if (pStep->opt_i1 == AS_PTN_1STEP) {
				pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_PLUS;
			}
			else if (pStep->opt_i1 == AS_PTN_2STEP_PN) {
				if (abs(pIO_Table->physics.PhPlane_t.z) > DEF_HPI) {
					pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_MINUS;
				}
				else {
					pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_PLUS;
				}
			}
			else;
			return status;
		}

		//High phase
		temp_chk1 = pStep->phase2 + pIO_Table->auto_ctrl.phase_chk_range[AS_SLEW_ID];
		temp_chk2 = pStep->phase2 - pIO_Table->auto_ctrl.phase_chk_range[AS_SLEW_ID];
		if (abs(pStep->phase2) > DEF_PI) {
			status = STEP_ERROR;
		}
		else if (temp_chk1 > DEF_PI) {
			if ((pIO_Table->physics.PhPlane_t.z < (temp_chk1 - DEF_2PI)) || (pIO_Table->physics.PhPlane_t.z > temp_chk2))
				status = STEP_FIN;
		}
		else if (temp_chk2 < -DEF_PI) {
			if ((pIO_Table->physics.PhPlane_t.z > (-temp_chk2 + DEF_2PI)) || (pIO_Table->physics.PhPlane_t.z < temp_chk1))
				status = STEP_FIN;
		}
		else {
			if ((pIO_Table->physics.PhPlane_t.z < temp_chk1) && (pIO_Table->physics.PhPlane_t.z > temp_chk2))
				status = STEP_FIN;
		}

		//Too small sway
		if (pIO_Table->physics.sway_amp_t_ph < g_spec.as_compl_swayLv[I_AS_LV_COMPLE]) {
			status = STEP_FIN;
		}

		if (pStep->act_counter > pStep->time_count) 
			status = STEP_ERROR;

		if ((status == STEP_FIN)||(status == STEP_ERROR)) {
			pIO_Table->auto_ctrl.as_start_ph[AS_SLEW_ID] = AS_START_HIGH_PHASE;
			if (pStep->opt_i1 == AS_PTN_1STEP) {
				pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_MINUS;
			}
			else if (pStep->opt_i1 == AS_PTN_2STEP_PN) {
				if (abs(pIO_Table->physics.PhPlane_t.z) > DEF_HPI) {
					pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_MINUS;
				}
				else {
					pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_PLUS;
				}
			}
			else;
			return status;
		}

	}break;
	case CTR_TYPE_ACC_AS: {
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_ACC_AS_2PN: {
		if (pStep->opt_i2 == pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID]) { //位置合わせ補正方向への加速時
			if (pStep->act_counter > pStep->time_count + pStep->opt_i1) {
				status = STEP_FIN;
			}
		}
		else {
			if (pStep->act_counter > pStep->time_count) {
				status = STEP_FIN;
			}
		}
	}break;
	case CTR_TYPE_DEC_V: {
		if (pStep->_v < 0.0) {
			if (auto_vref[MOTION_ID_SLEW] >= pStep->_v) status = STEP_FIN;
		}
		else if (pStep->_v > 0.0) {
			if (auto_vref[MOTION_ID_SLEW] <= pStep->_v) status = STEP_FIN;
		}
		else {
			if (abs(pIO_Table->physics.wth) < g_spec.slew_notch_spd[1] * 0.05) {
				status = STEP_FIN;
			}
		}
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_DEC_AS_2PN: {
		if (pStep->_v < 0.0) {
			if (pIO_Table->physics.wth >= pStep->_v) {
				status = STEP_FIN;
			}
		}
		else if (pStep->_v > 0.0) {
			if (pIO_Table->physics.wth <= pStep->_v) {
				status = STEP_FIN;
			}
		}
		else {
			if (abs(pIO_Table->physics.wth) < g_spec.slew_notch_spd[1] * 0.05) {//旋回判定は引込の1/10
				status = STEP_FIN;
			}
		}
		if (pStep->opt_i2 == pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID]) { //位置合わせ補正方向への加速時
			if (pStep->act_counter > pStep->time_count + pStep->opt_i1) {
				status = STEP_FIN;
			}
		}
		else {
			if (pStep->act_counter > pStep->time_count) {
				status = STEP_FIN;
			}
		}
	} break;
	case CTR_TYPE_CONST_V_TIME: {
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_BH_WAIT:
	case CTR_TYPE_SLEW_WAIT:
	case CTR_TYPE_MH_WAIT:
	case CTR_TYPE_ACC_TIME:
	case CTR_TYPE_ACC_V:
	case CTR_TYPE_ACC_TIME_OR_V:
	case CTR_TYPE_DEC_TIME:
	default:
		break;
	}
	return status;
}
int CPlayer::check_step_status_bh(LPST_MOTION_ELEMENT pStep) {
	int status = STEP_ON_GOING;

	switch (pStep->type) {
	case CTR_TYPE_TIME_WAIT: {
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_TIME_WAIT_2PN: {
		if (pStep->opt_i2 == AS_INIT_SWAY_SMALL) {
			if (pStep->act_counter > (pStep->time_count - pStep->opt_i1)) {//初期振れ分停止時間短縮
				status = STEP_FIN;
				if (pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] == AS_DIR_PLUS) {
					pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_MINUS;//移動方向切替
				}
				else {
					pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_PLUS;
				}
			}
		}
		else {
			if (pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] == AS_DIR_PLUS) {
				if (pIO_Table->auto_ctrl.as_start_ph[AS_BH_ID] == AS_START_LOW_PHASE) {
					if (pStep->act_counter > (pStep->time_count + pStep->opt_i1)) {//初期振れ分停止時間延長
						status = STEP_FIN;
						pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_MINUS;//移動方向切替
					}
				}
				else if (pIO_Table->auto_ctrl.as_start_ph[AS_BH_ID] == AS_START_HIGH_PHASE) {
					if (pStep->act_counter > (pStep->time_count - pStep->opt_i1)) {//初期振れ分停止時間短縮
						status = STEP_FIN;
						pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_MINUS;//移動方向切替
					}
				}
				else {
					if (pStep->act_counter > pStep->time_count) {
						status = STEP_FIN;
						pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_MINUS;//移動方向切替
					}
				}
			}
			else {
				if (pIO_Table->auto_ctrl.as_start_ph[AS_BH_ID] == AS_START_LOW_PHASE) {
					if (pStep->act_counter > (pStep->time_count - pStep->opt_i1)) {//初期振れ分停止時間短縮
						status = STEP_FIN;
						pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_PLUS;//移動方向切替
					}
				}
				else if (pIO_Table->auto_ctrl.as_start_ph[AS_BH_ID] == AS_START_HIGH_PHASE) {
					if (pStep->act_counter > (pStep->time_count + pStep->opt_i1)) {//初期振れ分停止時間延長
						status = STEP_FIN;
						pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_PLUS;//移動方向切替
					}
				}
				else {
					if (pStep->act_counter > pStep->time_count - pStep->opt_i1) {//初期振れ分停止時間短縮
						status = STEP_FIN;
						pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_PLUS;//移動方向切替
					}
				}
			}
		}
	}break;
	case CTR_TYPE_TIME_WAIT_2PP: {
		if (pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] == AS_DIR_PLUS) {
			if (pIO_Table->auto_ctrl.as_start_ph[AS_BH_ID] == AS_START_HIGH_PHASE) {
				if (pStep->act_counter > (pStep->time_count - pStep->opt_i1)) {//初期振れ分停止時間短縮
					status = STEP_FIN;
				}
			}
			else {
				if (pStep->act_counter > (pStep->time_count + pStep->opt_i1)) {//初期振れ分停止時間延長
					status = STEP_FIN;
				}
			}
		}
		else {
			if (pIO_Table->auto_ctrl.as_start_ph[AS_BH_ID] == AS_START_HIGH_PHASE) {
				if (pStep->act_counter > (pStep->time_count + pStep->opt_i1)) {//初期振れ分停止時間延長
					status = STEP_FIN;
				}
			}
			else {
				if (pStep->act_counter > (pStep->time_count - pStep->opt_i1)) {//初期振れ分停止時間短縮
					status = STEP_FIN;
				}
			}
		}
	}break;
	case CTR_TYPE_SINGLE_PHASE_WAIT: {
		double temp_chk1 = pStep->phase1 + pIO_Table->auto_ctrl.phase_chk_range[AS_BH_ID];
		double temp_chk2 = pStep->phase1 - pIO_Table->auto_ctrl.phase_chk_range[AS_BH_ID];

		//Low Phase
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

		//Too small sway
		if (pIO_Table->physics.sway_amp_n_ph < g_spec.as_compl_swayLv[I_AS_LV_COMPLE]) {
			status = STEP_FIN;
		}
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;

	}break;
	case CTR_TYPE_DOUBLE_PHASE_WAIT: {
		//Low phase
		double temp_chk1 = pStep->phase1 + pIO_Table->auto_ctrl.phase_chk_range[AS_BH_ID];
		double temp_chk2 = pStep->phase1 - pIO_Table->auto_ctrl.phase_chk_range[AS_BH_ID];

		if (abs(pStep->phase1) > DEF_PI) {
			status = STEP_ERROR;
		}
		else if (temp_chk1 > DEF_PI) {
			if ((pIO_Table->physics.PhPlane_n.z < (temp_chk1 - DEF_2PI)) || (pIO_Table->physics.PhPlane_n.z > temp_chk2))
				status = STEP_FIN;
		}
		else if (temp_chk2 < -DEF_PI) {
			if ((pIO_Table->physics.PhPlane_n.z > (-temp_chk2 + DEF_2PI)) || (pIO_Table->physics.PhPlane_n.z < temp_chk1))
				status = STEP_FIN;
		}
		else {
			if ((pIO_Table->physics.PhPlane_n.z < temp_chk1) && (pIO_Table->physics.PhPlane_n.z > temp_chk2))
				status = STEP_FIN;
		}

		if ((status == STEP_FIN)|| (status == STEP_ERROR)) {
			pIO_Table->auto_ctrl.as_start_ph[AS_BH_ID] = AS_START_LOW_PHASE;
			if (pStep->opt_i1 == AS_PTN_1STEP) {
				pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_PLUS;
			}
			else if (pStep->opt_i1 == AS_PTN_2STEP_PN) {
				if (abs(pIO_Table->physics.PhPlane_n.z) > DEF_HPI) {
					pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_MINUS;
				}
				else {
					pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_PLUS;
				}
			}
			else;
			return status;
		}

		//High phase
		temp_chk1 = pStep->phase2 + pIO_Table->auto_ctrl.phase_chk_range[AS_BH_ID];
		temp_chk2 = pStep->phase2 - pIO_Table->auto_ctrl.phase_chk_range[AS_BH_ID];

		if (abs(pStep->phase2) > DEF_PI) {
			status = STEP_ERROR;
		}
		else if (temp_chk1 > DEF_PI) {
			if ((pIO_Table->physics.PhPlane_n.z < (temp_chk1 - DEF_2PI)) || (pIO_Table->physics.PhPlane_n.z > temp_chk2))
				status = STEP_FIN;
		}
		else if (temp_chk2 < -DEF_PI) {
			if ((pIO_Table->physics.PhPlane_n.z > (-temp_chk2 + DEF_2PI)) || (pIO_Table->physics.PhPlane_n.z < temp_chk1))
				status = STEP_FIN;
		}
		else {
			if ((pIO_Table->physics.PhPlane_n.z < temp_chk1) && (pIO_Table->physics.PhPlane_n.z > temp_chk2))
				status = STEP_FIN;
		}

		//Too small sway
		if (pIO_Table->physics.sway_amp_n_ph < g_spec.as_compl_swayLv[I_AS_LV_COMPLE]) {
			status = STEP_FIN;
		}

		if (pStep->act_counter > pStep->time_count) {
			status = STEP_ERROR;
		}

		if ((status == STEP_FIN)||(status == STEP_ERROR)) {
			pIO_Table->auto_ctrl.as_start_ph[AS_BH_ID] = AS_START_HIGH_PHASE;
			if (pStep->opt_i1 == AS_PTN_1STEP) {
				pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_MINUS;
			}
			else if (pStep->opt_i1 == AS_PTN_2STEP_PN) {
				if (abs(pIO_Table->physics.PhPlane_n.z) > DEF_HPI) {
					pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_MINUS;
				}
				else {
					pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_PLUS;
				}
			}
			else;
			return status;
		}
	}break;
	case CTR_TYPE_ACC_AS: {
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_ACC_AS_2PN: {
		if (pStep->opt_i2 == pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID]) { //位置合わせ補正方向への加速時
			if (pStep->act_counter > pStep->time_count + pStep->opt_i1) {
				status = STEP_FIN;
			}
		}
		else {
			if (pStep->act_counter > pStep->time_count) {
				status = STEP_FIN;
			}
		}
	}break;
	case CTR_TYPE_DEC_V: {
		if(pStep->_v < 0.0){
			if(auto_vref[MOTION_ID_BH] >= pStep->_v) status = STEP_FIN;
		}
		else if (pStep->_v > 0.0){
			if (auto_vref[MOTION_ID_BH] <= pStep->_v) status = STEP_FIN;
		}
		else {
			if (abs(pIO_Table->physics.vR) < g_spec.bh_notch_spd[1] * 0.1) {
				status = STEP_FIN;
			}
		}
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_DEC_AS_2PN: {
		if (pStep->_v < 0.0) {
			if (auto_vref[MOTION_ID_BH] >= pStep->_v) status = STEP_FIN;
		}
		else if (pStep->_v > 0.0) {
			if (auto_vref[MOTION_ID_BH] <= pStep->_v) status = STEP_FIN;
		}
		else {
			if (abs(pIO_Table->physics.vR) < g_spec.bh_notch_spd[1] * 0.1) status = STEP_FIN;
		}

		if (pStep->opt_i2 == pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID]) { //位置合わせ補正方向への加速時
			if (pStep->act_counter > pStep->time_count + pStep->opt_i1) {
				status = STEP_FIN;
			}
		}
		else {
			if (pStep->act_counter > pStep->time_count) {
				status = STEP_FIN;
			}
		}
	} break;
	case CTR_TYPE_CONST_V_TIME: {
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_SLEW_WAIT: {
		if (pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] < CHECK_RANGE_OF_SLEW_POS/180.0*DEF_PI)  status = STEP_FIN;
	}break;
	case CTR_TYPE_BH_WAIT:
	case CTR_TYPE_MH_WAIT:
	case CTR_TYPE_ACC_TIME:
	case CTR_TYPE_ACC_V:
	case CTR_TYPE_ACC_TIME_OR_V:
	case CTR_TYPE_DEC_TIME:
	default:
		break;
	}
	return status;
}
int CPlayer::check_step_status_mh(LPST_MOTION_ELEMENT pStep) {
	int status = STEP_ON_GOING;

	switch (pStep->type) {
	case CTR_TYPE_TIME_WAIT: {
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_SINGLE_PHASE_WAIT: {
		double temp_chk1 = pStep->phase1 + pIO_Table->auto_ctrl.phase_chk_range[AS_MH_ID];
		double temp_chk2 = pStep->phase1 - pIO_Table->auto_ctrl.phase_chk_range[AS_MH_ID];

		if (temp_chk1 > DEF_PI) {
			if ((pIO_Table->physics.PhPlane_r.z < (temp_chk1 - DEF_2PI)) || (pIO_Table->physics.PhPlane_r.z > temp_chk2))
				status = STEP_FIN;
		}
		else if (temp_chk2 < -DEF_PI) {
			if ((pIO_Table->physics.PhPlane_r.z >(-temp_chk2 + DEF_2PI)) || (pIO_Table->physics.PhPlane_r.z < temp_chk1))
				status = STEP_FIN;
		}
		else {
			if ((pIO_Table->physics.PhPlane_r.z < temp_chk1) && (pIO_Table->physics.PhPlane_r.z > temp_chk2)&&(pIO_Table->physics.wPhPlane_r<0.0))
				status = STEP_FIN;
		}

		//Too small sway
		if (pIO_Table->physics.sway_amp_r_ph < g_spec.as_compl_swayLv[I_AS_LV_COMPLE]) {
			status = STEP_FIN;
		}
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;

	}break;
	case CTR_TYPE_DOUBLE_PHASE_WAIT: {

#if 0
		double temp_chk1 = pStep->phase1 + pIO_Table->auto_ctrl.phase_chk_range[AS_MH_ID];
		double temp_chk2 = pStep->phase1 - pIO_Table->auto_ctrl.phase_chk_range[AS_MH_ID];
		//LOW PHASE
		if (temp_chk1 > DEF_PI) {
			if ((pIO_Table->physics.PhPlane_r.z < (temp_chk1 - DEF_2PI)) || (pIO_Table->physics.PhPlane_r.z > temp_chk2))
				status = STEP_FIN;
		}
		else if (temp_chk2 < -DEF_PI) {
			if ((pIO_Table->physics.PhPlane_r.z >(-temp_chk2 + DEF_2PI)) || (pIO_Table->physics.PhPlane_r.z < temp_chk1))
				status = STEP_FIN;
		}
		else {
			if ((pIO_Table->physics.PhPlane_r.z < temp_chk1) && (pIO_Table->physics.PhPlane_r.z > temp_chk2))
				status = STEP_FIN;
		}
		//HIGH PHASE
		if (abs(pStep->phase2) >= DEF_PI) {
			status = STEP_ERROR;
		}
		else if (pStep->phase2 > 0.0) {
			temp_chk1 = pStep->phase2 - pIO_Table->auto_ctrl.phase_chk_range[AS_MH_ID];
			if (pIO_Table->physics.PhPlane_r.z > temp_chk1) {
				temp_chk2 = pStep->phase2 + pIO_Table->auto_ctrl.phase_chk_range[AS_MH_ID];
				if (temp_chk2 < DEF_PI) {
					if (pIO_Table->physics.PhPlane_r.z < temp_chk2) 
						status = STEP_FIN;
				}
				else {
					if (pIO_Table->physics.PhPlane_r.z < -DEF_2PI + temp_chk2) 
						status = STEP_FIN;
				}
			}
		}
		else {
			temp_chk1 = pStep->phase2 + pIO_Table->auto_ctrl.phase_chk_range[AS_MH_ID];
			if (pIO_Table->physics.PhPlane_r.z < temp_chk1) {
				temp_chk2 = pStep->phase2 - pIO_Table->auto_ctrl.phase_chk_range[AS_MH_ID];
				if (temp_chk2 > -DEF_PI) {
					if (pIO_Table->physics.PhPlane_r.z > temp_chk2) 
						status = STEP_FIN;
				}
				else {
					if (pIO_Table->physics.PhPlane_r.z > DEF_2PI - temp_chk2) 
						status = STEP_FIN;
				}
			}
		}
#endif
		//Too small sway
		if (pIO_Table->physics.sway_amp_r_ph < g_spec.as_compl_swayLv[I_AS_LV_COMPLE]) {
			status = STEP_FIN;
		}
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_ACC_AS: {
		//	if(abs(auto_vref[MOTION_ID_BH]) > abs(pStep->_v)) status = STEP_FIN;
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_DEC_V: {
		if (pStep->_v < 0.0) {
			if (auto_vref[MOTION_ID_MH] >= pStep->_v) status = STEP_FIN;
		}
		else if (pStep->_v > 0.0) {
			if (auto_vref[MOTION_ID_MH] <= pStep->_v) status = STEP_FIN;
		}
		else {
			if (abs(auto_vref[MOTION_ID_MH]) < g_spec.hoist_notch_spd[1] * 0.5) status = STEP_FIN;
		}
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_CONST_V_TIME: {
		if (pStep->act_counter > pStep->time_count) status = STEP_FIN;
	}break;
	case CTR_TYPE_SLEW_WAIT:
	case CTR_TYPE_BH_WAIT:
	case CTR_TYPE_MH_WAIT:
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

double CPlayer::act_slew_steps(LPST_MOTION_UNIT pRecipe) {
	double output_v = 0.0;
	LPST_MOTION_ELEMENT pStep = &pRecipe->motions[pRecipe->iAct];

	pStep->act_counter++;
	switch (pStep->type) {
	case CTR_TYPE_TIME_WAIT: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_TIME_WAIT_2PN: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_TIME_WAIT_2PP: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_SINGLE_PHASE_WAIT: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_DOUBLE_PHASE_WAIT: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_ACC_AS: {
		output_v = (double)pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * pStep->_v;
	}break;
	case CTR_TYPE_ACC_AS_2PN: {
		output_v = (double)pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] 
						*(pStep->_v + (double)(pStep->opt_i1 * inf.cycle_ms / 1000) * g_spec.bh_acc[FWD_ACC]);

	}break;
	case CTR_TYPE_DEC_V: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_DEC_AS_2PN: {
		output_v = pStep->_v;
	} break;
	case CTR_TYPE_CONST_V_TIME: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_BH_WAIT:
	case CTR_TYPE_SLEW_WAIT:
	case CTR_TYPE_MH_WAIT:
	case CTR_TYPE_ACC_TIME:
	case CTR_TYPE_ACC_V:
	case CTR_TYPE_ACC_TIME_OR_V:
	case CTR_TYPE_DEC_TIME:
	default: {
		return 0.0;
	}
	}
	if (STEP_FIN == check_step_status_slew(pStep)) pRecipe->iAct++;
	return output_v;
};
double CPlayer::act_bh_steps(LPST_MOTION_UNIT pRecipe) {
	double output_v = 0.0;
	LPST_MOTION_ELEMENT pStep = &pRecipe->motions[pRecipe->iAct];

	pStep->act_counter++;
	switch (pStep->type) {
	case CTR_TYPE_TIME_WAIT:
		output_v = pStep->_v;
		break;
	case CTR_TYPE_TIME_WAIT_2PN: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_TIME_WAIT_2PP: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_SINGLE_PHASE_WAIT: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_DOUBLE_PHASE_WAIT: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_ACC_AS: {
		output_v = (double)pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * pStep->_v;
	}break;
	case CTR_TYPE_ACC_AS_2PN: {
		output_v = (double)pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] 
			      * (pStep->_v + (double)(pStep->opt_i1 * inf.cycle_ms / 1000) * g_spec.bh_acc[FWD_ACC]);
	}break;
	case CTR_TYPE_DEC_V: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_DEC_AS_2PN: {
		output_v = pStep->_v;
	} break;
	case CTR_TYPE_CONST_V_TIME: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_SLEW_WAIT: {
		output_v = pStep->_v;
	}break;

	case CTR_TYPE_BH_WAIT:
	case CTR_TYPE_MH_WAIT:
	case CTR_TYPE_ACC_TIME:
	case CTR_TYPE_ACC_V:
	case CTR_TYPE_ACC_TIME_OR_V:
	case CTR_TYPE_DEC_TIME:
	default: {
		return 0.0;
	}
		break;
	}
	if (STEP_FIN == check_step_status_bh(pStep)) pRecipe->iAct++;
	return output_v;
};
double CPlayer::act_mh_steps(LPST_MOTION_UNIT pRecipe) {
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
	case CTR_TYPE_ACC_AS: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_DEC_V: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_CONST_V_TIME: {
		output_v = pStep->_v;
	}break;
	case CTR_TYPE_SLEW_WAIT: {
		output_v = pStep->_v;
	}break;

	case CTR_TYPE_BH_WAIT:

	case CTR_TYPE_MH_WAIT:

	case CTR_TYPE_ACC_TIME:
	case CTR_TYPE_ACC_V:
	case CTR_TYPE_ACC_TIME_OR_V:
	case CTR_TYPE_DEC_TIME:
	case CTR_TYPE_DEC_TIME_OR_V:
	default:
		break;
	}
	if (STEP_FIN == check_step_status_mh(pStep)) pRecipe->iAct++;

	return output_v;
};

//# Caliculate PC reference
void CPlayer::cal_auto_ref() {

	set_motion_receipe();

	if (pMode->antisway == OPE_MODE_AS_ON){
		//## normal derection
		if ((p_motion_ptn[BH_AXIS]->ptn_status == PTN_UNIT_FIN) || (p_motion_ptn[BH_AXIS]->ptn_status == PTN_NOTHING)) {//Any pattern not running
			auto_vref[MOTION_ID_BH] = 0.0;
		}
		else if (p_motion_ptn[BH_AXIS]->ptn_status == PTN_PAUSE) {
			auto_vref[MOTION_ID_BH] = 0.0;
		}
		else if (p_motion_ptn[BH_AXIS]->ptn_status == PTN_STANDBY) {
			p_motion_ptn[BH_AXIS]->ptn_status = PTN_ACTIVE;
		}
		else if (p_motion_ptn[BH_AXIS]->ptn_status == PTN_ACTIVE) {
			if (p_motion_ptn[BH_AXIS]->iAct >p_motion_ptn[BH_AXIS]->n_step) {
				p_motion_ptn[BH_AXIS]->ptn_status = PTN_UNIT_FIN;
				auto_vref[MOTION_ID_BH] = 0.0;
			}
			else if (p_motion_ptn[BH_AXIS]->axis_type != BH_AXIS) {
				p_motion_ptn[BH_AXIS]->ptn_status = PTN_NOTHING;
				auto_vref[MOTION_ID_BH] = 0.0;
			}
			else {
				auto_vref[MOTION_ID_BH] = act_bh_steps(p_motion_ptn[BH_AXIS]);
			}
		}
		else {
			auto_vref[MOTION_ID_BH] = 0.0;
		}
		//## tangent derection
		if ((p_motion_ptn[SLW_AXIS]->ptn_status == PTN_UNIT_FIN) || (p_motion_ptn[SLW_AXIS]->ptn_status == PTN_NOTHING)) {//Any pattern not running
				auto_vref[MOTION_ID_SLEW] = 0.0;
			}
		else if (p_motion_ptn[SLW_AXIS]->ptn_status == PTN_PAUSE) {
				auto_vref[MOTION_ID_SLEW] = 0.0;
			}
		else if (p_motion_ptn[SLW_AXIS]->ptn_status == PTN_STANDBY) {
				p_motion_ptn[SLW_AXIS]->ptn_status = PTN_ACTIVE;
			}
		else if (p_motion_ptn[SLW_AXIS]->ptn_status == PTN_ACTIVE) {
				if (p_motion_ptn[SLW_AXIS]->iAct >p_motion_ptn[SLW_AXIS]->n_step) {
					p_motion_ptn[SLW_AXIS]->ptn_status = PTN_UNIT_FIN;
					auto_vref[MOTION_ID_SLEW] = 0.0;
				}
				else if (p_motion_ptn[SLW_AXIS]->axis_type != SLW_AXIS) {
					p_motion_ptn[SLW_AXIS]->ptn_status = PTN_NOTHING;
					auto_vref[MOTION_ID_SLEW] = 0.0;
				}
				else {
					auto_vref[MOTION_ID_SLEW] = act_slew_steps(p_motion_ptn[SLW_AXIS]);
				}
			}
		else {
				auto_vref[MOTION_ID_SLEW] = 0.0;
			}
		//## hoist derection
		if ((p_motion_ptn[MH_AXIS]->ptn_status == PTN_UNIT_FIN) || (p_motion_ptn[MH_AXIS]->ptn_status == PTN_NOTHING)) {//Any pattern not running
			auto_vref[MOTION_ID_MH] = 0.0;
		}
		else if (p_motion_ptn[MH_AXIS]->ptn_status == PTN_PAUSE) {
			auto_vref[MOTION_ID_MH] = 0.0;
		}
		else if (p_motion_ptn[MH_AXIS]->ptn_status == PTN_STANDBY) {
			p_motion_ptn[MH_AXIS]->ptn_status = PTN_ACTIVE;
		}
		else if (p_motion_ptn[MH_AXIS]->ptn_status == PTN_ACTIVE) {
			if (p_motion_ptn[MH_AXIS]->iAct > p_motion_ptn[MH_AXIS]->n_step) {
				p_motion_ptn[MH_AXIS]->ptn_status = PTN_UNIT_FIN;
				auto_vref[MOTION_ID_MH] = 0.0;
			}
			else if (p_motion_ptn[MH_AXIS]->axis_type != MH_AXIS) {
				p_motion_ptn[MH_AXIS]->ptn_status = PTN_NOTHING;
				auto_vref[MOTION_ID_MH] = 0.0;
			}
			else {
				auto_vref[MOTION_ID_MH] = act_mh_steps(p_motion_ptn[MH_AXIS]);
			}
		}
		else {
			auto_vref[MOTION_ID_MH] = 0.0;
		}
	}
	else {
		auto_vref[MOTION_ID_SLEW] = 0.0;
		auto_vref[MOTION_ID_MH] = 0.0;
		auto_vref[MOTION_ID_BH] = 0.0;
	}
	

	CManager* pMan = (CManager*)VectpCTaskObj[g_itask.mng];
	if (pOrder->job_A.status == JOB_ORDER_STEP_ON_GOING) {
//		if ((p_motion_ptn[BH_AXIS]->ptn_status == PTN_UNIT_FIN) && (p_motion_ptn[SLW_AXIS]->ptn_status == PTN_UNIT_FIN)) {
		if ((pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] < g_spec.as_compl_nposLv[I_AS_LV_TRIGGER]) && (pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] < g_spec.as_compl_tposLv[I_AS_LV_TRIGGER])) {
			pMan->handle_order_event(ORDER_ID_JOB_A, JOB_EVENT_COMPLETE_COMMAND_STEP_NORMAL, pOrder->job_A.job_step_now);
		}
	}

	return ;
};

